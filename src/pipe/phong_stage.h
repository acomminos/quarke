#ifndef QUARKE_SRC_PIPE_PHONG_STAGE_H_
#define QUARKE_SRC_PIPE_PHONG_STAGE_H_

#include <glm/glm.hpp>
#include <glad/glad.h>

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
  // Instantiates a phong stage drawing to a light buffer.
  PhongStage(GLuint light_fbo, GLuint light_buffer, GLuint color_tex,
             GLuint normal_tex, GLuint depth_tex);

  // Accumulates the given point light's luminosity to the light buffer.
  void Illuminate(const PointLight& light);
 private:
  GLuint light_fbo_;
  GLuint light_buffer_;
  GLuint color_tex_;
  GLuint normal_tex_;
  GLuint depth_tex_;
};

}  // namespace pipe
}  // namespace quarke

#endif  // QUARKE_SRC_PIPE_PHONG_STAGE_H_
