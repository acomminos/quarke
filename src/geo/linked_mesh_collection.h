#ifndef QUARKE_SRC_GEO_LINKED_MESH_COLLECTION_H_
#define QUARKE_SRC_GEO_LINKED_MESH_COLLECTION_H_

#include <cassert>
#include "pipe/geometry_stage.h"
#include "geo/mesh.h"

namespace quarke {
namespace geo {

// A continuous linked list of meshes separated by material.
// Materials are not owned by the mesh collection, and are expected to be
// valid for the lifetime of the collection.
class LinkedMeshCollection {
 public:
  struct MaterialNode;
  struct MeshNode {
    std::unique_ptr<Mesh> mesh;
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
      : node_(node), next_(node ? node->start : nullptr) {}

    const Mesh* Next() override {
      // Iterate until the material changes.
      // We assume meshes of the same material are contiguous.
      if (!next_)
        return nullptr;
      assert(next_->material == node_);

      auto mesh = next_;
      next_ = next_ != node_->end ? next_->next.get() : nullptr;
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
    MaterialIterator(MaterialNode* start) : start_(start), next_(start), cur_iter_(nullptr) {}
    pipe::MaterialMeshIterator* NextMaterial() override {
      if (!next_)
        return nullptr;
      cur_iter_ = MaterialMeshIterator(next_);
      next_ = next_->next.get();
      return &cur_iter_;
    }

    void Reset() override {
      next_ = start_;
    }
   private:
    MaterialNode* start_;
    MaterialNode* next_;
    MaterialMeshIterator cur_iter_;
  };

  LinkedMeshCollection() : meshes_(nullptr), materials_(nullptr) {}

  void AddMesh(mat::Material* mat, std::unique_ptr<Mesh> mesh);

  MaterialIterator Iterator();
 private:

  // Gets the material node for the given material, or inserts a new material
  // node to the beginning of the material node list.
  MaterialNode* GetOrCreateMaterial(mat::Material* material);

  std::unique_ptr<MeshNode> meshes_;
  std::unique_ptr<MaterialNode> materials_;
};

}  // namespace geo
}  // namespace quarke

#endif  // QUARKE_SRC_GEO_LINKED_MESH_COLLECTION_H_
