#ifndef QUARKE_SRC_PIPE_SSAO_STAGE_H_
#define QUARKE_SRC_PIPE_SSAO_STAGE_H_

#include "pipe/fragment_stage.h"
#include "game/camera.h"
#include <memory>

namespace quarke {
namespace pipe {

// A screen-space ambient occlusion pass using a randomly rotated kernel.
// Designed to accept the final lit scene as input, but doesn't have to.
class SSAOStage {
 public:
  static std::unique_ptr<SSAOStage> Create(int width, int height);
  SSAOStage(std::unique_ptr<FragmentStage> fstage,
            GLint uniform_mvp_matrix,
            GLint uniform_light_tex,
            GLint uniform_depth_tex);

  void Clear();

  // Renders AO upon the given light_tex using information from depth_tex.
  // Assumes both textures are of type GL_TEXTURE_RECTANGLE.
  void Render(const game::Camera& camera, GLuint light_tex, GLuint depth_tex);

  GLuint fbo() const { return fstage_->fbo(); }
  GLuint buffer() const { return GL_COLOR_ATTACHMENT0; }
 private:
  std::unique_ptr<FragmentStage> fstage_;
  GLint uniform_mvp_matrix_;
  GLint uniform_light_tex_;
  GLint uniform_depth_tex_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_SSAO_STAGE_H_
