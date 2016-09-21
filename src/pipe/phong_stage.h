#ifndef QUARKE_SRC_PIPE_PHONG_STAGE_H_
#define QUARKE_SRC_PIPE_PHONG_STAGE_H_

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <memory>

namespace quarke {
namespace pipe {

struct PointLight {
  float intensity;
  glm::vec3 position;
  glm::vec3 color;
};

// A shader stage that additively blends point lights to a light buffer.
class PhongStage {
 public:
  // Creates a new phong stage based on color, normal, and depth buffers.
  static std::unique_ptr<PhongStage> Create(GLuint color_tex, GLuint normal_tex, GLuint depth_tex);

  // Instantiates a phong stage drawing to a light buffer.
  PhongStage(GLuint light_fbo, GLuint light_buffer, GLuint light_tex,
             GLuint color_tex, GLuint normal_tex, GLuint depth_tex);

  void Clear();

  // Accumulates the given point light's luminosity to the light buffer.
  void Illuminate(const PointLight& light);

  GLuint light_fbo() const { return light_fbo_; }
  GLuint light_buffer() const { return light_buffer_; }
  GLuint light_tex() const { return light_tex_; }

 private:
  GLuint light_fbo_;
  GLuint light_buffer_;
  GLuint light_tex_;

  GLuint color_tex_;
  GLuint normal_tex_;
  GLuint depth_tex_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_PHONG_STAGE_H_
