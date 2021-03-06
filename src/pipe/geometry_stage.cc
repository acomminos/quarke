#include "pipe/geometry_stage.h"
#include "mat/material.h"
#include "game/camera.h"
#include "geo/mesh.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

namespace quarke {
namespace pipe {

static const char* UNIFORM_MODEL_MATRIX_NAME = "model_matrix";
static const char* UNIFORM_MVP_MATRIX_NAME = "mvp_matrix";
static const char* UNIFORM_NORMAL_MATRIX_NAME = "normal_matrix";
static const char* UNIFORM_SAMPLER_MATRIX_NAME = "sampler";

static const GLuint FS_OUT_COLOR_BUFFER = 0;
static const GLuint FS_OUT_NORMAL_BUFFER = 1;
static const GLuint FS_OUT_POSITION_BUFFER = 2;

std::unique_ptr<GeometryStage> GeometryStage::Create(int width, int height) {
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  GLuint color_tex;
  glGenTextures(1, &color_tex);
  glBindTexture(GL_TEXTURE_RECTANGLE, color_tex);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, color_format(), width, height, 0,
               GL_RGBA, GL_FLOAT, nullptr);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_tex, 0);

  GLuint normal_tex;
  glGenTextures(1, &normal_tex);
  glBindTexture(GL_TEXTURE_RECTANGLE, normal_tex);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, normal_format(), width, height, 0,
               GL_RGBA, GL_FLOAT, nullptr);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normal_tex, 0);

  GLuint position_tex;
  glGenTextures(1, &position_tex);
  glBindTexture(GL_TEXTURE_RECTANGLE, position_tex);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, position_format(), width, height, 0,
               GL_RGBA, GL_FLOAT, nullptr);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, position_tex, 0);

  GLuint depth_tex;
  glGenTextures(1, &depth_tex);
  glBindTexture(GL_TEXTURE_2D, depth_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, depth_format(), width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "[gs] Incomplete framebuffer." << std::endl;
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &color_tex);
    glDeleteTextures(1, &normal_tex);
    glDeleteTextures(1, &position_tex);
    glDeleteTextures(1, &depth_tex);
    return nullptr;
  }

  return std::make_unique<GeometryStage>(width, height, fbo, color_tex,
                                         normal_tex, position_tex, depth_tex);
}

GeometryStage::GeometryStage(int width, int height, GLuint fbo,
                             GLuint color_tex, GLuint normal_tex,
                             GLuint position_tex, GLuint depth_tex)
  : out_width_(width), out_height_(height), fbo_(fbo), color_tex_(color_tex)
  , position_tex_(position_tex), normal_tex_(normal_tex), depth_tex_(depth_tex)
{}

void GeometryStage::Clear() {
  // TODO: scoped framebuffer state
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
  glDrawBuffers(3, (const GLenum[]) { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GeometryStage::SetOutputSize(int width, int height) {
  out_width_ = width;
  out_height_ = height;

  glBindTexture(GL_TEXTURE_RECTANGLE, color_tex_);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, color_format(), width, height, 0,
               GL_RGBA, GL_FLOAT, nullptr);
  glBindTexture(GL_TEXTURE_RECTANGLE, normal_tex_);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, normal_format(), width, height, 0,
               GL_RGBA, GL_FLOAT, nullptr);
  glBindTexture(GL_TEXTURE_2D, depth_tex_);
  glTexImage2D(GL_TEXTURE_2D, 0, depth_format(), width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
}

void GeometryStage::Render(const game::Camera& camera, MaterialIterator& iter,
                           bool color, bool normal, bool position) {
  if (camera.viewport_width() != out_width_ ||
      camera.viewport_height() != out_height_)
  {
    // TODO: this doesn't manage camera switching very well.
    //       perhaps we shouldn't reuse stages with different cameras.
    SetOutputSize(camera.viewport_width(), camera.viewport_height());
  }

  glEnable(GL_DEPTH_TEST);
  // scoped framebuffer setting? that's an extra GL call.
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
  // note:
  // - fragment varying outputs correspond to a color index.
  // - glDrawBuffers specifies which buffers are to be mapped to each index
  // - they're adjacent and monotonic, curious.
  // - want to use glBindFragDataLocation to map to one of these buffers.
  // having draw buffers bound here is sort of a reasonable assumption, since
  // we define their locations internal to this class.
  const GLenum buffers[] = {
    color ? GL_COLOR_ATTACHMENT0 : (GLenum) GL_NONE,
    normal ? GL_COLOR_ATTACHMENT1 : (GLenum) GL_NONE,
    position ? GL_COLOR_ATTACHMENT2 : (GLenum) GL_NONE
  };
  glDrawBuffers(3, buffers);

  glm::mat4 vp_matrix = camera.ComputeProjection();
  glm::mat4 view_matrix = camera.ComputeView();

  MaterialMeshIterator* mit = nullptr;
  while ((mit = iter.NextMaterial()) != nullptr) {
    // FIXME: should we be using a pointer to index materials?
    mat::Material* mat = mit->Material();
    auto cache_it = shader_cache_.find(mat);
    GLuint program;
    if (cache_it == shader_cache_.end()) {
      // Construct a new shader for `mat`.
      program = glCreateProgram();
      GLuint vs = BuildVertexShader(*mat);
      GLuint fs = BuildFragmentShader(*mat);
      glAttachShader(program, vs);
      glAttachShader(program, fs);
      glLinkProgram(program);
      shader_cache_[mat] = program;

      // TODO: check for errors generated here.
      //       we should also clean up unused material shaders after a few
      //       idle passes. but hey, we have like no materials so life's good.
    } else {
      program = cache_it->second;
    }

    glUseProgram(program);

    mat->OnBindProgram(program);

    GLuint model_location = glGetUniformLocation(program, UNIFORM_MODEL_MATRIX_NAME);
    GLuint mvp_location = glGetUniformLocation(program, UNIFORM_MVP_MATRIX_NAME);
    GLuint normal_matrix_location = glGetUniformLocation(program, UNIFORM_NORMAL_MATRIX_NAME);

    const geo::Mesh* mesh = nullptr;
    while ((mesh = mit->Next()) != nullptr) {
      geo::VertexBuffer& vb = mesh->array_buffer();
      glBindVertexArray(vb.vertex_array());

      glm::mat4 model_matrix = mesh->transform();
      glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
      glm::mat4 mvp_matrix = vp_matrix * mesh->transform();
      glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp_matrix));
      glm::mat4 normal_matrix = glm::transpose(glm::inverse(mesh->transform()));
      glUniformMatrix4fv(normal_matrix_location, 1, GL_FALSE, glm::value_ptr(normal_matrix));

      mat->PreDrawMesh(*mesh); // setup per-mesh uniform attributes

      glDisable(GL_BLEND);
      glDrawArrays(GL_TRIANGLES, 0, mesh->num_vertices());

      mat->PostDrawMesh(*mesh);
    }

    mat->OnUnbindProgram(program);
  }
}

GLuint GeometryStage::BuildVertexShader(const mat::Material& material) const {
  // XXX: currently, materials are unused.
  //      should populate vertex/uniform info from them.
  std::ostringstream vs;
  vs << "#version 330" << std::endl;

  vs << "uniform mat4 " << UNIFORM_MODEL_MATRIX_NAME << ";" << std::endl;
  vs << "uniform mat4 " << UNIFORM_MVP_MATRIX_NAME << ";" << std::endl;
  vs << "uniform mat4 " << UNIFORM_NORMAL_MATRIX_NAME << ";" << std::endl;

  vs << "layout(location = " << geo::VertexBuffer::VS_ATTRIB_POSITION << ") "
     << "in vec3 position;" << std::endl;
  vs << "layout(location = " << geo::VertexBuffer::VS_ATTRIB_NORMAL << ") "
     << "in vec3 normal;" << std::endl;
  if (material.use_texture()) {
    vs << "layout(location = " << geo::VertexBuffer::VS_ATTRIB_TEXCOORD << ") "
       << "in vec2 texcoord;" << std::endl;
  }

  // XXX: do we even want to deal with color vs. texturing here?
  //      we could probably delegate this to material implementations.
  vs << "out vec4 vNormal;" << std::endl;
  vs << "out vec4 vPosition;" << std::endl;

  // Source from material to provide the material() function
  if (material.has_vertex_shader()) {
    material.BuildVertexShader(vs);
  }

  vs << "void main(void) {" << std::endl
     << "vNormal = normalize(" << UNIFORM_NORMAL_MATRIX_NAME << " * vec4(normal, 0.0));" << std::endl
     << "vPosition = " << UNIFORM_MODEL_MATRIX_NAME << " * vec4(position, 1.0);" << std::endl;

  vs << "gl_Position = " << UNIFORM_MVP_MATRIX_NAME << " * vec4(position, 1.0);" << std::endl;

  // Call the generated material() function from the VS
  if (material.has_vertex_shader()) {
    vs << "material();" << std::endl;
  }

  vs << "}";

#ifdef QUARKE_DEBUG
  std::cout << "[gs] geometry stage generated vertex shader:"
            << std::endl << vs.str() << std::endl;
#endif  // QUARKE_DEBUG

  GLuint shader = glCreateShader(GL_VERTEX_SHADER);
  std::string str = vs.str();
  GLint length = str.length();
  const char* c_str = str.c_str();
  glShaderSource(shader, 1, &c_str, &length);
  glCompileShader(shader);
  // XXX error check
  return shader;
}

GLuint GeometryStage::BuildFragmentShader(const mat::Material& material) const {
  std::ostringstream fs;
  fs << "#version 330" << std::endl;

  fs << "uniform mat4 " << UNIFORM_MVP_MATRIX_NAME << ";" << std::endl;
  fs << "uniform mat4 " << UNIFORM_NORMAL_MATRIX_NAME << ";" << std::endl;

  fs << "in vec4 vNormal;" << std::endl;
  fs << "in vec4 vPosition;" << std::endl;

  fs << "layout(location = " << FS_OUT_COLOR_BUFFER << ") "
     << "out vec4 outColor;" << std::endl;
  fs << "layout(location = " << FS_OUT_NORMAL_BUFFER << ") "
     << "out vec4 outNormal;" << std::endl;
  fs << "layout(location = " << FS_OUT_POSITION_BUFFER << ") "
     << "out vec4 outPosition;" << std::endl;

  // call upon dat material to make frags
  material.BuildFragmentShader(fs);

  fs << "void main(void) {" << std::endl
     << "outColor = vec4(abs(cos(vPosition.x)), abs(cos(vPosition.y)), abs(cos(vPosition.z)), 1.0); //vColor;" << std::endl
     << "outNormal = normalize(vec4(vNormal.xyz, 0.0));" << std::endl
     << "outPosition = vPosition;" << std::endl
     << "material();" << std::endl
     << "}";

#ifdef QUARKE_DEBUG
  std::cout << "[gs] geometry stage generated fragment shader:"
            << std::endl << fs.str() << std::endl;
#endif  // QUARKE_DEBUG

  GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
  std::string str = fs.str();
  GLint length = str.length();
  const char* c_str = str.c_str();
  glShaderSource(shader, 1, &c_str, &length);
  glCompileShader(shader);
  // XXX error check
  return shader;
}

}  // namespace pipe
}  // namespace quarke
