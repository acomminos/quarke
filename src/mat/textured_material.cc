#include "mat/textured_material.h"
#include "geo/mesh.h"

namespace quarke {
namespace mat {

const GLuint TEXTURE_SLOT = 0;

static const std::string VS_SOURCE = R"(
out vec2 vTexcoord;

void material(void) {
  vTexcoord = texcoord;
}
)";

static const std::string FS_SOURCE = R"(
uniform sampler2D tex;

in vec2 vTexcoord;

void material(void) {
  // XXX: Currently, we don't support transparent objects using multipass.
  outColor = vec4(texture(tex, vTexcoord).xyz, 1.0);
}
)";

TexturedMaterial::TexturedMaterial(GLenum target, GLuint texture)
  : target_(target), texture_(texture), texture_location_(-1) {
}

void TexturedMaterial::BuildVertexShader(std::ostream& vs) const {
  vs << VS_SOURCE;
}

void TexturedMaterial::BuildFragmentShader(std::ostream& fs) const {
  fs << FS_SOURCE;
}

void TexturedMaterial::OnBindProgram(GLuint program) {
  if (texture_location_ == -1) {
    texture_location_ = glGetUniformLocation(program, "tex");
    glUniform1f(texture_location_, TEXTURE_SLOT);
  }
}

void TexturedMaterial::PreDrawMesh(const geo::Mesh& mesh) {
  // TODO: bind to mesh's assigned texture
  glActiveTexture(GL_TEXTURE0 + TEXTURE_SLOT);
  glBindTexture(target_, texture_);
}

}  // namespace mat
}  // namespace quarke
