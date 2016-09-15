#include "game/scene.h"

namespace quarke {
namespace game {

Scene::Scene(int width, int height)
  : camera_(width, height) {
}

void Scene::Render() {
  if (!geom_) {
    // TODO: instantiate this elsewhere where we can handle failures.
    //       in addition, make the mesh interface somewhat exposed.
    geom_ = std::make_unique<pipe::GeometryStage>();
  }
}

void Scene::OnResize(int width, int height) {
  camera_.SetViewport(width, height);
}

}  // namespace game
}  // namespace quarke
