#ifndef QUARKE_SRC_PIPE_OVERLAY_STAGE_H_
#define QUARKE_SRC_PIPE_OVERLAY_STAGE_H_

#include "pipe/fragment_stage.h"

namespace quarke {
namespace pipe {

// A stage blitting a 2D overlay to the screen.
class OverlayStage {
 public:
  // Creates a new bitmap overlay stage with the given width, height, and
  // texture format.
  static std::unique_ptr<OverlayStage> Create(int width, int height);

  OverlayStage(std::unique_ptr<FragmentStage> fstage, GLuint texture);

  // Uploads a texture onto the GPU for display in the overlay stage.
  // The uploaded texture must be the same size as the one the overlay stage is
  // initialized with.
  void Upload(const GLvoid* data, int width, int height, GLenum format,
              GLenum type);

  // Updates a region of the overlay of size (width, height) read to the texture
  // at (offset_x, offset_y). Invalid if the region updated is not within the
  // bounds of the original upload.
  void UpdateRegion(const GLvoid* data, int offset_x, int offset_y, int width,
                    int height, GLenum format, GLenum type);

  // Blends the overlay on top of the given framebuffer.
  void Draw(GLuint fbo, GLuint buffer);
 private:
  std::unique_ptr<FragmentStage> fstage_;
  GLint uniform_texture_overlay_;
  GLuint texture_overlay_;
};

};
};

#endif  // QUARKE_SRC_PIPE_OVERLAY_STAGE_H_
