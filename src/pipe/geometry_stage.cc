#include "pipe/geometry_stage.h"
#include "mat/material.h"
#include "game/camera.h"
#include "geo/mesh.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

namespace quarke {
namespace pipe {

static const char*  UNIFORM_MVP_MATRIX_NAME = "mvp_matrix";

static const GLuint VS_IN_POSITION_LOCATION = 0;
static const GLuint VS_IN_NORMAL_LOCATION = 1;
static const GLuint VS_IN_TEXCOORD_LOCATION = 2;

static const GLuint FS_OUT_COLOR_BUFFER = 0;
static const GLuint FS_OUT_NORMAL_BUFFER = 1;

void GeometryStage::Clear() {
  // TODO: scoped framebuffer state
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
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
  glBindTexture(GL_TEXTURE_RECTANGLE, depth_tex_);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, depth_format(), width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
}

void GeometryStage::Render(const game::Camera& camera, MeshIterator& iter) {
  if (camera.viewport_width() != out_width_ ||
      camera.viewport_height() != out_height_)
  {
    // TODO: this doesn't manage camera switching very well.
    //       perhaps we shouldn't reuse stages with different cameras.
    SetOutputSize(camera.viewport_width(), camera.viewport_height());
  }

  // scoped framebuffer setting? that's an extra GL call, homie.
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
  // note:
  // - fragment varying outputs correspond to a color index.
  // - glDrawBuffers specifies which buffers are to be mapped to each index
  // - they're adjacent and monotonic, curious.
  // - want to use glBindFragDataLocation to map to one of these buffers.
  // having draw buffers bound here is sort of a reasonable assumption, since
  // we define their locations internal to this class.
  glDrawBuffers(2, (const GLenum[]) { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 });

  glm::mat4 vp_matrix = camera.Compute();

  MaterialIterator* mit = nullptr;
  while ((mit = iter.Next()) != nullptr) {
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
      //       vram ain't a thang
    } else {
      program = cache_it->second;
    }

    glUseProgram(program);

    mat->OnBindProgram(program);

    GLuint mvp_location = glGetUniformLocation(program, UNIFORM_MVP_MATRIX_NAME);

    const geo::Mesh* mesh = nullptr;
    while ((mesh = mit->Next()) != nullptr) {
      // TODO: assert that mesh::material == material here

      // Here's the floorplan here;
      // - bind vertex attributes
      // - apply per-model transform
      // - do we have any per-model uniforms? if so, major league swagout.

      // oh shit, I guess we could sort of use a pointer to a material as an
      // identifier (as disappointing as that is)
      glEnableVertexAttribArray(VS_IN_POSITION_LOCATION);
      switch (mesh->array_buffer_format()) {
        case geo::VertexFormat::P3N3T2:
          glEnableVertexAttribArray(VS_IN_NORMAL_LOCATION);
          glEnableVertexAttribArray(VS_IN_TEXCOORD_LOCATION);
          break;
        case geo::VertexFormat::P3N3:
          glEnableVertexAttribArray(VS_IN_NORMAL_LOCATION);
          break;
        case geo::VertexFormat::P3T2:
          glEnableVertexAttribArray(VS_IN_TEXCOORD_LOCATION);
          break;
      }

      glm::mat4 mvp_matrix = vp_matrix * mesh->transform();
      glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp_matrix));
    }

    mat->OnUnbindProgram(program);
  }
}

GLuint GeometryStage::BuildVertexShader(const mat::Material& material) const {
  // XXX: currently, materials are unused.
  //      should populate vertex/uniform info from them.
  std::ostringstream vs;
  vs << "#version 150" << std::endl;

  vs << "uniform mat4 " << UNIFORM_MVP_MATRIX_NAME << ";";

  vs << "layout(location = " << VS_IN_POSITION_LOCATION << ")"
     << "in vec4 position;";
  vs << "layout(location = " << VS_IN_NORMAL_LOCATION << ")"
     << "in vec4 normal;";
  vs << "layout(location = " << VS_IN_TEXCOORD_LOCATION << ")"
     << "in vec2 texcoord;";

  // XXX: do we even want to deal with color vs. texturing here?
  //      we could probably delegate this to material implementations.
  //      let's not get ahead of ourselves though with plumbing.
  //      ( ͡° ͜ʖ ͡°)
  vs << "out vec4 vColor;";
  vs << "out vec4 vNormal;";
  vs << "out vec2 vTexcoord;";

  // XXX: default placeholder material
  // TODO: source from material here
  // material.BuildVertexShader(position, normal, texcoord)

  vs << "void main(void) {"
     // XXX: hwhite test
     << "vColor = vec4(1.0, 1.0, 1.0, 1.0);"
     << "vNormal = normal;"
     << "vTexcoord = texcoord;"
     << "gl_Position = " << UNIFORM_MVP_MATRIX_NAME << " * position;"
     << "}";

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
  fs << "#version 150" << std::endl;

  fs << "uniform mat4 " << UNIFORM_MVP_MATRIX_NAME << ";";

  // TODO: populate material's texture (if applicable)
  //fs << "uniform sampler2D texture;";

  fs << "in vec4 vColor";
  fs << "in vec4 vNormal";
  fs << "in vec4 vTexcoord";

  fs << "layout(location = " << FS_OUT_COLOR_BUFFER << ")"
     << "out vec4 outColor";
  fs << "layout(location = " << FS_OUT_NORMAL_BUFFER << ")"
     << "out vec4 outNormal";

  // XXX: default placeholder material (again)
  // call upon dat material to make frags
  fs << "void main(void) {"
     << "outColor = vColor;"
     << "outNormal = vNormal;"
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
