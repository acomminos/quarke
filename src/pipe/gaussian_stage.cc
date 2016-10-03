#include "pipe/gaussian_stage.h"

namespace quarke {
namespace pipe {

static const char* FS_SOURCE = R"(
#version 330 core

uniform sampler2DRect tex;
uniform float sigma;

layout(location = 0) out vec4 out_color;

void main(void) {
  int size = int(floor(3.0 * sigma));
  vec4 value = vec4(0.0, 0.0, 0.0, 0.0);
  float d = 2.0 * pow(sigma, 2.0);
  // TODO: abuse separability of gaussian.
  //       alternatively, use preconstructed convolution filter.
  //       tradeoff between time and space, as usual.
  float sum = 0.0;
  for (int y = -size; y <= size; y++) {
    for (int x = -size; x <= size; x++) {
      float weight = exp(-((pow(x, 2.0) / d) + (pow(y, 2.0) / d)));
      value += weight * texture(tex, vec2(x, y) + gl_FragCoord.xy);
      sum += weight;
    }
  }
  out_color = value / sum; // normalize
}
)";

std::unique_ptr<GaussianStage> GaussianStage::Create(int width, int height) {
  auto fstage = FragmentStage::Create(width, height, 1, FS_SOURCE);
  if (!fstage) {
    return nullptr;
  }
  return std::make_unique<GaussianStage>(std::move(fstage));
}

GaussianStage::GaussianStage(std::unique_ptr<FragmentStage> fstage)
  : fstage_(std::move(fstage)) {
  uniform_texture_ = glGetUniformLocation(fstage_->program(), "tex");
  uniform_sigma_ = glGetUniformLocation(fstage_->program(), "sigma");
}

void GaussianStage::Render(GLuint texture, GLfloat sigma) {
  glUseProgram(fstage_->program());
  glUniform1f(uniform_sigma_, sigma);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_RECTANGLE, texture);
  glUniform1i(uniform_sigma_, 0);

  fstage_->Clear(0.f, 0.f, 0.f, 0.f);
  fstage_->Draw();
}

}  // namespace pipe
}  // namespace quarke
