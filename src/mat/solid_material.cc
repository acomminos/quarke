#include "mat/solid_material.h"

namespace quarke {
namespace mat {

void SolidMaterial::BuildVertexShader(std::ostream& vs) {
  vs << "uniform vec4 solidColor;" << std::endl;
  vs << "void main() {";
  vs << "outColor = solidColor;";
  vs << "}";
}

void SolidMaterial::BuildFragmentShader(std::ostream& fs) {
}

void SolidMaterial::OnBindProgram(GLuint program) {

}

void SolidMaterial::OnUnbindProgram(GLuint program) {

}

}  // namespace mat
}  // namespace quarke
