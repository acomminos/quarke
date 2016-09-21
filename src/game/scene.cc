#include "game/scene.h"
#include "mat/solid_material.h"

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

  // FIXME: render with basic material for now.
  mat::SolidMaterial basicMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0));
  StubMaterialIterator iter(&basicMaterial, meshes_);
  geom_->Render(camera_, iter);
}

void Scene::OnResize(int width, int height) {
  camera_.SetViewport(width, height);
}

}  // namespace game
}  // namespace quarke
