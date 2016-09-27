#include "pipe/ambient_stage.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace quarke {
namespace pipe {

static const char* FS_SOURCE = R"(
#version 330

uniform vec4 ambientColor;
uniform sampler2DRect albedo;

layout(location = 0) out vec4 outColor;

void main(void) {
  outColor = ambientColor * texture(albedo, gl_FragCoord.xy);
}

)";

std::unique_ptr<AmbientStage> AmbientStage::Create(int width, int height,
                                                   const glm::vec4 ambient_color) {
  auto fstage = FragmentStage::Create(width, height, 1, FS_SOURCE);
  if (!fstage) {
    std::cerr << "Failed to create ambient fragment stage!" << std::endl;
    return nullptr;
  }
  return std::make_unique<AmbientStage>(std::move(fstage), ambient_color);
}

AmbientStage::AmbientStage(std::unique_ptr<FragmentStage> fstage,
                           const glm::vec4 ambient_color)
  : fstage_(std::move(fstage)), ambient_color_(ambient_color) {
  ambient_color_location_ = glGetUniformLocation(fstage_->program(), "ambientColor");
  albedo_location_ = glGetUniformLocation(fstage_->program(), "albedo");
}

void AmbientStage::Clear() {
  fstage_->Clear(0.0, 0.0, 0.0, 0.0);
}

void AmbientStage::Render(GLuint albedo_tex) {
  const GLuint TEX_UNIT = 0;
  glUseProgram(fstage_->program());
  glUniform1i(albedo_location_, 0);
  glActiveTexture(GL_TEXTURE0 + TEX_UNIT);
  glBindTexture(GL_TEXTURE_RECTANGLE, albedo_tex);
  glUniform4fv(ambient_color_location_, 1, glm::value_ptr(ambient_color_));
  fstage_->Draw();
}

}  // namespace pipe
}  // namespace quarke
