#ifndef QUARKE_SRC_MAT_TEXTURED_MATERIAL_H_
#define QUARKE_SRC_MAT_TEXTURED_MATERIAL_H_

#include "mat/material.h"

namespace quarke {
namespace mat {

// A material that paints a 2D texture onto uv-mapped meshes.
class TexturedMaterial : public Material {
 public:
  // TODO: accept a per-mesh texture instead
  TexturedMaterial(GLenum target, GLuint texture);

  void BuildVertexShader(std::ostream& vs) const override;
  void BuildFragmentShader(std::ostream& fs) const override;
  bool has_vertex_shader() const override { return true; }
  bool use_texture() const override { return true; }

  void OnBindProgram(GLuint program) override;
  void PreDrawMesh(const geo::Mesh& mesh) override;
 private:
  GLenum target_;
  GLuint texture_;
  GLint texture_location_;
};

}  // namespace mat
}  // namespace quarke

#endif  // QUARKE_SRC_MAT_TEXTURED_MATERIAL_H_
