#ifndef QUARKE_SRC_GAME_SCENE_H_
#define QUARKE_SRC_GAME_SCENE_H_

#include <map>
#include <memory>
#include <list>
#include "game/camera.h"
#include "geo/mesh.h"
#include "mat/solid_material.h"
#include "pipe/ambient_stage.h"
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
  //       or move into separate pipeline class?
  std::unique_ptr<pipe::GeometryStage> geom_;
  std::unique_ptr<pipe::AmbientStage> ambient_;
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

// A continuous linked list of meshes separated by material.
// Materials are not owned by the mesh collection, and are expected to be
// valid for the lifetime of the collection.
class LinkedMeshCollection {
 public:
  struct MaterialNode;
  struct MeshNode {
    std::unique_ptr<geo::Mesh> mesh;
    std::unique_ptr<MeshNode> next;
    MaterialNode* material;
  };

  struct MaterialNode {
    mat::Material* material;
    MeshNode* start;
    MeshNode* end;
    std::unique_ptr<MaterialNode> next;
  };

  class MaterialMeshIterator : public pipe::MaterialMeshIterator {
   public:
    MaterialMeshIterator(MaterialNode* node)
      : node_(node) {}

    const geo::Mesh* Next() override {
      // Iterate until the material changes.
      // We assume meshes of the same material are contiguous.
      if (!next_ || next_ == node_->end)
        return nullptr;
      assert(next_->material == node_);

      auto mesh = next_;
      next_ = next_->next.get();
      return mesh->mesh.get();
    }

    mat::Material* Material() override {
      return node_->material;
    }
   private:
    MaterialNode* node_;
    MeshNode* next_;
  };

  class MaterialIterator : public pipe::MaterialIterator {
   public:
    MaterialIterator(MaterialNode* next) : next_(next), cur_iter_(nullptr) {}
    pipe::MaterialMeshIterator* NextMaterial() override {
      if (!next_)
        return nullptr;
      auto next = next_;
      next_ = next_->next.get();
      cur_iter_ = MaterialMeshIterator(next);
      return &cur_iter_;
    }
   private:
    MaterialNode* next_;
    MaterialMeshIterator cur_iter_;
  };

  LinkedMeshCollection() : meshes_(nullptr), materials_(nullptr) {}

  void AddMesh(mat::Material* mat, std::unique_ptr<geo::Mesh> mesh);

  MaterialIterator Iterator();
 private:

  // Gets the material node for the given material, or inserts a new material
  // node to the beginning of the material node list.
  MaterialNode* GetOrCreateMaterial(mat::Material* material);

  std::unique_ptr<MeshNode> meshes_;
  std::unique_ptr<MaterialNode> materials_;
};

}  // namespace game
}  // namespace quarke

#endif  // QUARKE_SRC_GAME_SCENE_H_
