#include "mat/solid_material.h"
#include "geo/mesh.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace quarke {
namespace mat {

SolidMaterial::SolidMaterial(glm::vec4 color) : color_(color), color_location_(-1) { }

void SolidMaterial::BuildVertexShader(std::ostream& vs) const {
}

void SolidMaterial::BuildFragmentShader(std::ostream& fs) const {
  fs << "uniform vec4 solidColor;" << std::endl
     << "void material() {" << std::endl
     << "outColor = solidColor;" << std::endl
     << "}" << std::endl;
}

void SolidMaterial::OnBindProgram(GLuint program) {
  if (color_location_ == -1) {
    color_location_ = glGetUniformLocation(program, "solidColor");
  }
}

void SolidMaterial::PreDrawMesh(const geo::Mesh& mesh) {
  glUniform4fv(color_location_, 1, glm::value_ptr(mesh.color()));
}

}  // namespace mat
}  // namespace quarke
