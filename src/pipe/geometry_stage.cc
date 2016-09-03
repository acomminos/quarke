#include "pipe/geometry_stage.h"
#include "geo/mesh.h"

namespace quarke {
namespace pipe {

static const std::string vs_header = R"(
  in vec4 position;
)";

static const std::string fs_header = R"(
  layout(position = 0) out vec3 color;
  layout(position = 1) out vec3 normal;

)";

void GeometryStage::DrawMesh(const geo::Mesh& mesh) {
  // TODO:
  // - bind texture
  // - draw triangle strips

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
  // note:
  // - fragment varying outputs correspond to a color index.
  // - glDrawBuffers specifies which buffers are to be mapped to each index
  // - they're adjacent and monotonic, curious.
  // - want to use glBindFragDataLocation to map to one of these buffers.
  glDrawBuffers(2, (const GLenum[]) { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 });
}

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

}  // namespace pipe
}  // namespace quarke
