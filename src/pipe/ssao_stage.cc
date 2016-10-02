#include "pipe/ssao_stage.h"
#include <glm/gtc/type_ptr.hpp>

namespace quarke {
namespace pipe {

static const char* FS_SOURCE = R"(
#version 330 core

uniform mat4 mvp_matrix;
uniform sampler2DRect light_tex;
uniform sampler2DRect depth_tex;

layout(location = 0) out vec4 outColor;

const int filterRadius = 2;
const int samples = 4;

// Undoes the MVP transformation from screen space to clip space.
float linearizeDepth(in float depth) {
  float z = depth * 2.0 - 1.0;
  return -(z - mvp_matrix[2][3]) / mvp_matrix[2][2];
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void) {
  // use a parametric box filter, because gaussians are slow
  float val = 0.0;
  float centerDepth = linearizeDepth(texture(depth_tex, gl_FragCoord.xy).r);
  for (int i = 0; i < samples; i++) {
    vec2 coords = filterRadius * vec2(rand(gl_FragCoord.xy + i),
                                      rand(gl_FragCoord.xy - i));
    float depth = texture(depth_tex, gl_FragCoord.xy + coords).r;
    val += linearizeDepth(depth);
  }
  val /= samples;

  float shadow = 0.1 * max(centerDepth - val, 0.0);
  outColor = texture(light_tex, gl_FragCoord.xy) - (vec4(1.0, 1.0, 1.0, 0.0) * shadow);
}

)";

std::unique_ptr<SSAOStage> SSAOStage::Create(int width, int height) {
  auto fstage = FragmentStage::Create(width, height, 1, FS_SOURCE);
  if (!fstage)
    return nullptr;

  GLint u_mvp_matrix = glGetUniformLocation(fstage->program(), "mvp_matrix");
  GLint u_light_tex = glGetUniformLocation(fstage->program(), "light_tex");
  GLint u_depth_tex = glGetUniformLocation(fstage->program(), "depth_tex");

  return std::make_unique<SSAOStage>(std::move(fstage), u_mvp_matrix,
                                     u_light_tex, u_depth_tex);
}

SSAOStage::SSAOStage(std::unique_ptr<FragmentStage> fstage,
                     GLint uniform_mvp_matrix,
                     GLint uniform_light_tex,
                     GLint uniform_depth_tex)
  : fstage_(std::move(fstage))
  , uniform_mvp_matrix_(uniform_mvp_matrix)
  , uniform_light_tex_(uniform_light_tex)
  , uniform_depth_tex_(uniform_depth_tex) {}

void SSAOStage::Clear() {
  fstage_->Clear(0.f, 0.f, 0.f, 0.f);
}

void SSAOStage::Render(const game::Camera& camera, GLuint light_tex, GLuint depth_tex) {
  glUseProgram(fstage_->program());

  auto proj = camera.ComputeProjection();
  glUniformMatrix4fv(uniform_mvp_matrix_, 1, GL_FALSE, glm::value_ptr(proj));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_RECTANGLE, light_tex);
  glUniform1i(uniform_light_tex_, 0);

  // FIXME: for some reason we can't use GL_TEXTURE_RECTANGLE here to disable compare mode :(
  //        we should probably switch to npot GL_TEXTURE_2D everywhere anyway.
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, depth_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
  glUniform1i(uniform_depth_tex_, 1);

  fstage_->Draw();
}

}  // namespace pipe
}  // namespace quarke
