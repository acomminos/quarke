#ifndef QUARKE_SRC_MAT_SOLID_MATERIAL_H_
#define QUARKE_SRC_MAT_SOLID_MATERIAL_H_

#include "mat/material.h"

namespace quarke {
namespace mat {

// A material with a solid colour.
class SolidMaterial : public Material {
 public:
  void BuildVertexShader(std::ostream& vs) override;
  void BuildFragmentShader(std::ostream& fs) override;
  void OnBindProgram(GLuint program) override;
  void OnUnbindProgram(GLuint program) override;
};

}  // namespace mat
}  // namespace quarke

#endif  // QUARKE_SRC_MAT_SOLID_MATERIAL_H_
