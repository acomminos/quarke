#include "game/scene.h"
#include "mat/solid_material.h"

namespace quarke {
namespace game {

Scene::Scene(int width, int height)
  : camera_(width, height) {
  // XXX: testing
  meshes_.push_back(geo::Mesh::FromOBJ("../model/teapot.obj"));
}

void Scene::Render() {
  if (!geom_) {
    // TODO: instantiate this elsewhere where we can handle failures.
    //       in addition, make the mesh interface somewhat exposed.
    geom_ = pipe::GeometryStage::Create(camera_.viewport_width(),
                                        camera_.viewport_height());
    assert(geom_);
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
