#include "pipe/stage.h"

namespace quarke {
namespace pipe {

/* static */
std::unique_ptr<Stage> Stage::Initialize(GLuint vs, GLuint fs) {
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  glLinkProgram(prog);

  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  GLuint color_tex;
  glGenTextures(1, &color_tex);
  // TODO: load data
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_tex, 0);

  GLuint depth_tex;
  glGenTextures(1, &

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    goto cleanup;
  }

cleanup:
  glDeleteTextures(1, &color_tex);
  glDeleteFramebuffers(1, &fbo);
  glDeleteProgram(1, &prog);
  return nullptr;
}

Stage::Stage(GLuint prog) : prog_(prog), invalid_(true) {

}

}  // namespace pipe
}  // namespace quarke
