#include "geo/linked_mesh_collection.h"

namespace quarke {
namespace geo {

void LinkedMeshCollection::AddMesh(mat::Material* mat,
                                   std::unique_ptr<Mesh> mesh) {
  auto mat_node = GetOrCreateMaterial(mat);
  auto mesh_node = std::make_unique<LinkedMeshCollection::MeshNode>();
  mesh_node->mesh = std::move(mesh);
  mesh_node->material = mat_node;

  if (mat_node->end) {
    assert(mat_node->start);
    mesh_node->next = nullptr;
    mat_node->end->next = std::move(mesh_node);
    mat_node->end = mat_node->end->next.get();
  } else {
    assert(!mat_node->start);
    // This must be a new material.
    mat_node->start = mesh_node.get();
    mat_node->end = mesh_node.get();
    mesh_node->next = std::move(meshes_);
    meshes_ = std::move(mesh_node);
  }
}

LinkedMeshCollection::MaterialIterator
LinkedMeshCollection::Iterator() {
  return MaterialIterator(materials_.get());
}

LinkedMeshCollection::MaterialNode*
LinkedMeshCollection::GetOrCreateMaterial(mat::Material* mat) {
  MaterialNode* node = materials_.get();
  while (node) {
    if (node->material == mat)
      return node;
    node = node->next.get();
  }
  auto new_node = std::make_unique<LinkedMeshCollection::MaterialNode>();
  new_node->material = mat;
  new_node->start = nullptr;
  new_node->end = nullptr;
  new_node->next = std::move(materials_);
  materials_ = std::move(new_node);
  return materials_.get();
}

}  // namespace geo
}  // namespace quarke
