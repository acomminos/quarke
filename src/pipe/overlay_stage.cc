#include "pipe/overlay_stage.h"
#include <iostream>

namespace quarke {
namespace pipe {

using std::make_unique;
using std::move;
using std::unique_ptr;

static const char* FS_SOURCE = R"(
#version 330 core

uniform sampler2D u_overlay_tex;

out vec4 out_color;

void main() {
  out_color = texture2DRect(gl_FragCoord.xy);
}
)";

/* static */
unique_ptr<OverlayStage> Create(int width, int height) {
  auto fstage = FragmentStage::Create(width, height, 1, FS_SOURCE);
  if (!fstage) {
    std::cerr << "Failed to compile overlay fragment stage." << std::endl;
    return nullptr;
  }

  GLuint texture;
  glGenTextures(1, &texture);

  return make_unique<OverlayStage>(move(fstage), texture);
}

OverlayStage::OverlayStage(unique_ptr<FragmentStage> fstage, GLuint texture)
    : fstage_(move(fstage)), texture_overlay_(texture) {
  uniform_texture_overlay_ = glGetUniformLocation(fstage->program(), "u_overlay_tex");
}

void OverlayStage::Draw(GLuint fbo, GLuint buffer) {
}

}  // namespace pipe
}  // namespace quarke
