#ifndef QUARKE_SRC_GAME_CAMERA_H_
#define QUARKE_SRC_GAME_CAMERA_H_

#include <glm/glm.hpp>

namespace quarke {
namespace game {

// A simple implementation of a perspective projection camera.
// A camera stores a position, rotation, and parameters for perspective projection.
class Camera {
 public:
  Camera(int viewport_width, int viewport_height,
         float fov = 45.0, float near = 0.1, float far = 1000.0);

  // Sets the camera's view matrix using a given eye location, target location,
  // and an up vector.
  void LookAt(const glm::vec3 position, const glm::vec3 target,
              const glm::vec3 up);

  // Applies a translation to the view matrix in the basis formed by the matrix.
  void PostTranslate(const glm::vec3 translation);

  // Applies a translation in the standard basis prior to any transforms.
  void PreTranslate(const glm::vec3 translation);

  void SetViewport(int width, int height);

  // Computes a world-to-NDC matrix representing this camera.
  glm::mat4 Compute() const;

  int viewport_width() const { return viewport_width_; }
  int viewport_height() const { return viewport_height_; }
 private:
  // Invalidates the projection matrix, recomputing from camera settings.
  glm::mat4& InvalidateProjection();

  glm::mat4 view_;
  glm::mat4 projection_;

  int viewport_width_;
  int viewport_height_;
  float fov_;
  float near_;
  float far_;
};

}  // namespace game
}  // namespace quarke

#endif  // QUARKE_SRC_GAME_CAMERA_H_
