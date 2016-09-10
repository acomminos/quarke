#include "game/scene.h"

#include <glm/gtc/matrix_transform.hpp>

namespace quarke {
namespace game {

Scene::Scene() : fov_(45.0), near_(0.1), far_(1000.0) {
  // XXX: tmp
}

void Scene::Render() {
  // TODO
}

void Scene::OnResize(int width, int height) {
  viewport_width_ = width;
  viewport_height_ = height;
  projection_ = glm::perspective(fov_, (float) width / (float) height, near_,
                                 far_);
  // TODO
}

}  // namespace game
}  // namespace quarke
