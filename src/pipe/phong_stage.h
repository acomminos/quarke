#ifndef QUARKE_SRC_PIPE_PHONG_STAGE_H_
#define QUARKE_SRC_PIPE_PHONG_STAGE_H_

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <memory>

namespace quarke {

namespace game {
class Camera;
}  // namespace game

namespace pipe {

// FIXME: a temporary encapsulation of a point light source.
struct PointLight {
  float intensity;
  glm::vec3 position;
  glm::vec3 color;
};

// A shader stage that additively blends point lights to a light buffer.
class PhongStage {
 public:
  // Creates a new phong stage based on color, normal, position, and depth buffers.
  static std::unique_ptr<PhongStage> Create(int width, int height,
                                            GLuint color_tex, GLuint normal_tex,
                                            GLuint position_tex);

  // Instantiates a phong stage drawing to a light buffer.
  PhongStage(int width, int height, GLuint light_fbo, GLuint light_buffer,
             GLuint light_tex, GLuint screen_vbo, GLuint color_tex,
             GLuint normal_tex, GLuint position_tex, GLuint depth_tex);

  void Clear();

  // Accumulates the given point light's luminosity to the light buffer.
  void Illuminate(const game::Camera& camera, const PointLight& light);

  // Resizes the light buffer to the given dimensions.
  // Implicitly clears the light buffer.
  void Resize(int width, int height);

  GLuint light_fbo() const { return light_fbo_; }
  GLuint light_buffer() const { return light_buffer_; }
  GLuint light_tex() const { return light_tex_; }
  static GLuint light_format() { return GL_RGBA; }

 private:
  void BuildShaderProgram();

  int out_width_;
  int out_height_;

  GLuint light_fbo_;
  GLuint light_buffer_;
  GLuint light_tex_;
  GLuint screen_vbo_;
  GLuint screen_vao_;

  GLuint color_tex_;
  GLuint normal_tex_;
  GLuint position_tex_;
  GLuint depth_tex_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_PHONG_STAGE_H_
