#ifndef QUARKE_SRC_PIPE_STAGE_H_
#define QUARKE_SRC_PIPE_STAGE_H_

#include <GLFW/glfw3.h>
#include <memory>
#include <string>

namespace quarke {
namespace pipe {

// A rendering stage in the quarke shader pipeline. Stage implementations
// are typically provided with an input "pipe" from an earlier stage.
class Stage {
 public:
  // Renders the stage. After this is called, all outputs are to be considered
  // valid. Does nothing if the shader's outputs are already valid.
  void Render();

  // Invalidates the shader, requiring it to be re-rendered.
  void Invalidate() { invalid_ = true; }

 protected:
  // Subclass render implementation, sets up the shader inputs and calls a
  // glDraw function. Implementations should expect the stage output to be
  // set to its output texture, and all inputs to be bound.
  virtual void RenderImpl() = 0;

  bool invalid_;
};

// A link between shader stages, intended to allow hierarchical resolution.
// The provider of a TexturePipe should guarantee sanity of the texture.
class TexturePipe {
 friend Stage;
 public:
  TexturePipe(Stage& source, GLuint texture, GLenum format) :
    source_(source), texture_(texture), format_(format) {}

  // Calls the owner to render into the texture iff it is undefined.
  GLuint Resolve() {
    source_.Render();
    return texture_;
  }

  GLenum format() { return format_; }
 private:
  Stage& source_;
  GLuint texture_;
  GLenum format_;
};

// Some helpers to enforce static binding between stages.
struct RGBATexturePipe : public TexturePipe {
 public:
  RGBATexturePipe(Stage& source, GLuint texture) :
    TexturePipe(source, texture, GL_RGBA) {}
};

struct DepthTexturePipe : public TexturePipe {
 public:
  DepthTexturePipe(Stage& source, GLuint texture) :
    TexturePipe(source, texture, GL_DEPTH_COMPONENT) {}
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_STAGE_H_
