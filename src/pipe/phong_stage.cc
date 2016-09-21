#include "pipe/phong_stage.h"

namespace quarke {
namespace pipe {

void PhongStage::Illuminate(const PointLight& light) {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fbo_);
}

}  // namespace pipe
}  // namespace quarke
