#include "pipe/phong_stage.h"

namespace quarke {
namespace pipe {

std::unique_ptr<PhongStage> PhongStage::Create(GLuint color_tex,
                                               GLuint normal_tex,
                                               GLuint depth_tex) {
  return std::make_unique<PhongStage>(0, 0, 0, color_tex, normal_tex, depth_tex);
}

void PhongStage::Clear() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fbo_);
  glDrawBuffers(1, { light_buffer_ });
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void PhongStage::Illuminate(const PointLight& light) {
}

}  // namespace pipe
}  // namespace quarke
