#ifndef QUARKE_SRC_PIPE_FRAGMENT_STAGE_H_
#define QUARKE_SRC_PIPE_FRAGMENT_STAGE_H_

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <memory>
#include <vector>

namespace quarke {
namespace pipe {

// A generic shader stage that produces output textures in screen space.
// Textures are guaranteed to be of format GL_RGBA32F, bound to GL_TEXTURE_RECTANGLE.
// A depth buffer is present, but unused.
class FragmentStage {
 public:
  static std::unique_ptr<FragmentStage> Create(int width, int height,
                                               GLsizei num_outputs,
                                               const char* fs_source);

  FragmentStage(int width, int height, GLuint program, GLuint fbo,
                std::vector<GLuint> textures, std::vector<GLenum> buffers,
                GLsizei num_outputs, GLuint depth_tex, GLuint vbo, GLuint vao);
  ~FragmentStage();

  // Resizes the light buffer to the given dimensions.
  // Implicitly clears the light buffer.
  void Resize(int width, int height);

  void Clear(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

  // Calls glDrawArrays to draw a quad over screen coordinates.
  void Draw();

  GLuint program() const { return program_; }
  GLuint fbo() const { return fbo_; }
  GLuint texture(GLsizei idx) const {
    assert(idx >= 0 && idx < GL_MAX_COLOR_ATTACHMENTS);
    return textures_[idx];
  }
  GLuint depth_tex() { return depth_tex_; }
  static GLuint format() { return GL_RGBA32F; }
  static GLuint depth_format() { return GL_DEPTH_COMPONENT; }

 private:
  static bool BuildShaderProgram(GLuint& out_program, const char* fs_source);

  int out_width_;
  int out_height_;

  std::vector<GLuint> textures_;
  std::vector<GLenum> buffers_;
  GLsizei num_outputs_;

  const GLuint depth_tex_;
  const GLuint program_;
  const GLuint fbo_;
  const GLuint vbo_;
  const GLuint vao_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_PHONG_STAGE_H_
