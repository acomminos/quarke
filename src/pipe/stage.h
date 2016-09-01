#ifndef QUARKE_SRC_PIPE_STAGE_H_
#define QUARKE_SRC_PIPE_STAGE_H_

#include <memory>
#include <string>

namespace quarke {

// A rendering stage in the quarke shader pipeline. A stage is defined as a
// shader program taking textures as input and outputting 4-component floating
// point data to a texture.
class Stage {
 public:
  // Initializes the stage with the given GLSL shader source.
  // Returns the created stage on success, otherwise returns null.
  static std::unique_ptr<Stage> Initialize(const std::string& shader);

  void SetInputTexture(const std::string& input, ) final;

  // Renders the stage to a texture.
  virtual void Render() = 0;

  GLuint texture() { return tex_; }

 protected:
  Stage();
  ~Stage();

  GLuint prog_;
  GLuint fs_;
  GLuint vs_;

  GLuint fbo_;
  GLuint tex_;
};

}  // namespace quarke
