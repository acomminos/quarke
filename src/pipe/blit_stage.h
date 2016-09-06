#ifndef QUARKE_SRC_PIPE_BLIT_STAGE_H_
#define QUARKE_SRC_PIPE_BLIT_STAGE_H_

namespace quarke {
namespace pipe {

// A terminal pipeline stage that blits to the default framebuffer.
class BlitStage {
 public:
  BlitStage(RGBATexturePipe& texture) : texture_(texture);
  void Blit();
 private:
  RGBATexturePipe& texture_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_BLIT_STAGE_H_
