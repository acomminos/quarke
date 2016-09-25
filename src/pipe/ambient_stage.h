#ifndef QUARKE_SRC_PIPE_AMBIENT_STAGE_H_
#define QUARKE_SRC_PIPE_AMBIENT_STAGE_H_

#include <memory>
#include "pipe/fragment_stage.h"

namespace quarke {
namespace pipe {

// A stage that adds ambient lighting to the albedo buffer.
// TODO: This could potentially share an output buffer with the phong stage.
class AmbientStage {
 public:
  static std::unique_ptr<AmbientStage> Create(int width, int height,
                                              const glm::vec4 ambient_color = glm::vec4(0.1, 0.1, 0.1, 1.0));

  AmbientStage(std::unique_ptr<FragmentStage> fstage,
               const glm::vec4 ambient_color);

  void Clear();
  void Render(GLuint albedo_tex);

  void SetAmbientColor(const glm::vec4 color);

  GLuint ambient_tex() { return fstage_->texture(0); }
  GLuint ambient_fbo() { return fstage_->fbo(); }
  GLuint ambient_buffer() { return GL_COLOR_ATTACHMENT0; }
 private:
  glm::vec4 ambient_color_;
  std::unique_ptr<FragmentStage> fstage_;
  GLint ambient_color_location_;
  GLint albedo_location_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_AMBIENT_STAGE_H_
