#ifndef QUARKE_SRC_PIPE_BLIT_STAGE_H_
#define QUARKE_SRC_PIPE_BLIT_STAGE_H_

namespace quarke {
namespace pipe {

// A pipeline terminal stage that blits to the default framebuffer.
class BlitStage {
 public:
  void SetInputTexture(GLuint tex);
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_BLIT_STAGE_H_
