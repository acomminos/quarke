#include "game/camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace quarke {
namespace game {

Camera::Camera(int viewport_width, int viewport_height, float fov,
               float near, float far)
  : viewport_width_(viewport_width)
  , viewport_height_(viewport_height)
  , fov_(fov)
  , near_(near)
  , far_(far) {
  InvalidateProjection();
}

void Camera::LookAt(const glm::vec3 position, const glm::vec3 target,
                    const glm::vec3 up) {
  view_ = glm::lookAt(position, target, up);
}

void Camera::PostTranslate(const glm::vec3 translation) {
  view_ = glm::translate(glm::mat4(), translation) * view_;
}

void Camera::SetViewport(int width, int height) {
  viewport_width_ = width;
  viewport_height_ = height;
  InvalidateProjection();
}

glm::mat4 Camera::ComputeProjection() const {
  return projection_ * view_;
}

glm::mat4 Camera::ComputeView() const {
  return view_;
}

glm::vec3 Camera::Position() const {
  return glm::vec3(view_[3]) * -1.f;
}

glm::mat4& Camera::InvalidateProjection() {
  projection_ = glm::perspective(fov_,
                                 (float)viewport_width_
                                 / (float)viewport_height_,
                                 near_, far_);
  return projection_;
}

}  // namespace game
}  // namespace quarke
