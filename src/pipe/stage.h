#ifndef QUARKE_SRC_PIPE_STAGE_H_
#define QUARKE_SRC_PIPE_STAGE_H_

#include <GLFW/glfw3.h>
#include <memory>
#include <string>

namespace quarke {
namespace pipe {

// A rendering stage in the quarke shader pipeline. A stage is defined as a
// shader program taking textures as input and outputting floating RGBA data
// to a texture.
class Stage {
 public:
  // Initializes the stage with the given GLSL shaders.
  // Returns the created stage on success, otherwise returns null.
  static std::unique_ptr<Stage> Initialize(GLuint vs, GLuint fs);

  // Binds a texture from a shader stage as input.
  void SetInput(const std::string& name, GLuint texture) final;

  // Renders the stage to a texture.
  void Render();

  // Invalidates the shader, requiring it to be re-rendered.
  void Invalidate() final { invalid_ = true; }

  GLuint texture() { return tex_; }
  GLuint texture_format() { return GL_RGBA; }

 protected:
  Stage();
  ~Stage();

  // Subclass render implementation, sets up the shader inputs and calls a
  // glDraw function. Implementations should expect the stage output to be
  // set to its output texture, and all inputs to be bound.
  virtual void RenderImpl() = 0;

  bool invalid_;

  GLuint prog_;
  GLuint fs_;
  GLuint vs_;

  GLuint fbo_;
  GLuint color_tex_;
  GLuint depth_tex_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_STAGE_H_
