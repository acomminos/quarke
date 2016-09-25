#ifndef QUARKE_SRC_MAT_SOLID_MATERIAL_H_
#define QUARKE_SRC_MAT_SOLID_MATERIAL_H_

#include <glm/glm.hpp>
#include "mat/material.h"

namespace quarke {
namespace mat {

// A material with a solid colour.
class SolidMaterial : public Material {
 public:
  SolidMaterial();
  void BuildVertexShader(std::ostream& vs) const override;
  void BuildFragmentShader(std::ostream& fs) const override;
  void OnBindProgram(GLuint program) override;
  void PreDrawMesh(const geo::Mesh& mesh) override;
  bool has_vertex_shader() const override { return false; }
  bool use_texture() const override { return false; }
 private:
  GLint color_location_;
};

}  // namespace mat
}  // namespace quarke

#endif  // QUARKE_SRC_MAT_SOLID_MATERIAL_H_
