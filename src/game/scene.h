#ifndef QUARKE_SRC_GAME_SCENE_H_
#define QUARKE_SRC_GAME_SCENE_H_

#include <map>
#include <memory>
#include <list>
#include "game/camera.h"
#include "geo/mesh.h"
#include "mat/solid_material.h"
#include "pipe/geometry_stage.h"
#include "pipe/phong_stage.h"

namespace quarke {
namespace game {

// A scene manages the rendering world state (such as the projection matrix),
// as well as the gfx pipeline.
class Scene {
 public:
  Scene(int width, int height);

  void Update(float dt);
  void Render();

  // Called when the engine has resized the scene.
  // The dimensions provided are in device pixel units.
  void OnResize(int width, int height);

 private:
  Camera camera_;
  float rot; // tmp
  GLuint pepper_tex_; // tmp

  // TODO: should we put the pipeline here?
  std::unique_ptr<pipe::GeometryStage> geom_;
  std::unique_ptr<pipe::PhongStage> lighting_;

  typedef std::list<std::unique_ptr<geo::Mesh>> MeshPtrVector;

  MeshPtrVector meshes_;
  MeshPtrVector textured_meshes_;

  // TODO: A temporary material iterator until we have different material types implemented.
  class StubMaterialIterator : public pipe::MaterialIterator,
                               public pipe::MaterialMeshIterator {
   public:
    StubMaterialIterator(mat::Material* material, MeshPtrVector& meshes) :
      material_(material), meshes_(meshes), iterated_(false) {
      mesh_iter_ = meshes.cbegin();
    }

    pipe::MaterialMeshIterator* NextMaterial() override {
      if (iterated_)
        return nullptr;

      iterated_ = true;
      return this;
    }

    const geo::Mesh* Next() override {
      if (mesh_iter_ == meshes_.end())
        return nullptr;
      return (mesh_iter_++)->get();
    }

    mat::Material* Material() override {
      return material_;
    }

   private:
    mat::Material* const material_;
    const MeshPtrVector& meshes_;
    MeshPtrVector::const_iterator mesh_iter_;
    bool iterated_;
  };
};

}  // namespace game
}  // namespace quarke

#endif  // QUARKE_SRC_GAME_SCENE_H_
