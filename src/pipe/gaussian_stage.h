#ifndef QUARKE_SRC_PIPE_GAUSSIAN_STAGE_H_
#define QUARKE_SRC_PIPE_GAUSSIAN_STAGE_H_

#include "pipe/fragment_stage.h"

namespace quarke {
namespace pipe {

// A fragment stage applying a parametrically discretized gaussian to an RGBA
// texture. The 2D gaussian convolution encapsulates 99.7% (3σ) of the data.
class GaussianStage {
 public:
  static std::unique_ptr<GaussianStage> Create(int width, int height);

  GaussianStage(std::unique_ptr<FragmentStage> fstage);

  void Render(GLuint texture, GLfloat sigma);

  GLuint fbo() const { return fstage_->fbo(); }
  GLuint buffer() const { return GL_COLOR_ATTACHMENT0; }
  GLuint tex() const { return fstage_->texture(0); }
 private:
  std::unique_ptr<FragmentStage> fstage_;
  GLint uniform_texture_;
  GLint uniform_sigma_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_GAUSSIAN_STAGE_H_
