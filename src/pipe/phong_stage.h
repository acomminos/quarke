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
  float max_distance; // maximum reach of effect of a light
  glm::vec3 position;
  glm::vec4 color;
};

// A shader stage that additively blends point lights to a light buffer.
class PhongStage {
 public:
  // Creates a new phong stage based on color, normal, position, and depth buffers.
  static std::unique_ptr<PhongStage> Create(int width, int height,
                                            GLuint color_tex, GLuint normal_tex,
                                            GLuint position_tex, GLuint depth_tex);

  // Instantiates a phong stage drawing to a light buffer.
  PhongStage(int width, int height, GLuint program,
             GLuint light_fbo, GLuint light_buffer,
             GLuint light_tex, GLuint light_depth_tex, GLuint screen_vbo,
             GLuint screen_vao, GLuint color_tex, GLuint normal_tex,
             GLuint position_tex, GLuint depth_tex);

  void Clear();

  // Accumulates the given point light's luminosity to the light buffer.
  // FIXME: remove hackish shadow map thrown in; migrate to its own stage?
  void Illuminate(const game::Camera& camera, const PointLight& light,
                  GLuint shadow_cubemap);

  // Resizes the light buffer to the given dimensions.
  // Implicitly clears the light buffer.
  void Resize(int width, int height);

  GLuint fbo() const { return light_fbo_; }
  GLuint buffer() const { return light_buffer_; }
  GLuint tex() const { return light_tex_; }
  static GLuint format() { return GL_RGBA; }

 private:
  static bool BuildShaderProgram(GLuint& out_program);

  int out_width_;
  int out_height_;

  const GLuint program_;

  const GLuint light_fbo_;
  const GLuint light_buffer_;
  const GLuint light_tex_;
  const GLuint light_depth_tex_;
  const GLuint screen_vbo_;
  const GLuint screen_vao_;

  const GLuint color_tex_;
  const GLuint normal_tex_;
  const GLuint position_tex_;
  const GLuint depth_tex_;

  GLuint color_sampler_location_;
  GLuint normal_sampler_location_;
  GLuint position_sampler_location_;
  GLuint depth_sampler_location_;
  GLuint eye_position_location_;
  GLuint light_position_location_;
  GLuint light_color_location_;
  GLuint light_distance_location_;
  GLuint shadow_sampler_location_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_PHONG_STAGE_H_
