#include "mat/solid_material.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace quarke {
namespace mat {

SolidMaterial::SolidMaterial(glm::vec4 color) : color_(color) { }

void SolidMaterial::BuildVertexShader(std::ostream& vs) const {
  vs << "uniform vec4 solidColor;" << std::endl;
  vs << "void main() {";
  vs << "outColor = solidColor;";
  vs << "}";
}

void SolidMaterial::BuildFragmentShader(std::ostream& fs) const {
}

void SolidMaterial::OnBindProgram(GLuint program) {
  GLuint colorLocation = glGetUniformLocation(program, "solidColor");
  glUniform4fv(colorLocation, 4, glm::value_ptr(color_));
}

void SolidMaterial::OnUnbindProgram(GLuint program) {

}

}  // namespace mat
}  // namespace quarke
