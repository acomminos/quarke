#include "pipe/fragment_stage.h"
#include <iostream>

namespace quarke {
namespace pipe {

// a triangle fan covering ndc
static const GLfloat SCREEN_VERTICES[] = {
  // (x, y)
  1.0, 1.0,
  -1.0, 1.0,
  -1.0, -1.0,
  1.0, -1.0,
};

static const GLuint VS_IN_POSITION_LOCATION = 0;

static const char* VS_SOURCE = R"(
#version 330

layout(location = 0) in vec2 position;

void main(void) {
  gl_Position = vec4(position, 0.0, 1.0);
}
)";

std::unique_ptr<FragmentStage> FragmentStage::Create(int width, int height,
    GLsizei num_outputs, const char* fs_source) {

  if (num_outputs > 4) {
    std::cerr << "Warning: attempted to create fragment stage with " << num_outputs << "outputs." << std::endl
              << "The GL spec only mandates 4 color attachments." << std::endl;
  }

  GLint max_attachments;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_attachments);
  assert(num_outputs <= max_attachments);

  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  std::vector<GLuint> textures(num_outputs);
  std::vector<GLuint> buffers(num_outputs);
  glGenTextures(num_outputs, textures.data());

  for (int i = 0; i < num_outputs; i++) {
    GLuint tex = textures[i];
    glBindTexture(GL_TEXTURE_RECTANGLE, tex);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, format(), width, height, 0,
                 GL_RGBA, GL_FLOAT, nullptr);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, tex, 0);
    buffers[i] = GL_COLOR_ATTACHMENT0 + i;
  }

  GLuint depth_tex;
  glGenTextures(1, &depth_tex);
  glBindTexture(GL_TEXTURE_RECTANGLE, depth_tex);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "[fs] Incomplete framebuffer." << std::endl;
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(num_outputs, textures.data());
    glDeleteTextures(1, &depth_tex);
    return nullptr;
  }

  GLuint program;
  if (!BuildShaderProgram(program, fs_source)) {
    std::cerr << "[fs] Building shader program failed." << std::endl;
    return nullptr;
  }

  GLuint screen_vbo;
  glGenBuffers(1, &screen_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(SCREEN_VERTICES), SCREEN_VERTICES, GL_STATIC_DRAW);

  GLuint screen_vao;
  glGenVertexArrays(1, &screen_vao);
  glBindVertexArray(screen_vao);
  glEnableVertexAttribArray(VS_IN_POSITION_LOCATION);
  glVertexAttribPointer(VS_IN_POSITION_LOCATION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(GLfloat) * 2, nullptr);

  return std::make_unique<FragmentStage>(width, height, program, fbo, textures,
                                         buffers, num_outputs,
                                         screen_vbo, screen_vao);
}

FragmentStage::FragmentStage(int width, int height, GLuint program, GLuint fbo,
                             std::vector<GLuint> textures, std::vector<GLenum> buffers,
                             GLsizei num_outputs, GLuint vbo, GLuint vao)
  : out_width_(width), out_height_(height), program_(program), fbo_(fbo)
  , textures_(textures), buffers_(buffers), num_outputs_(num_outputs)
  , vbo_(vbo), vao_(vao) {
}

FragmentStage::~FragmentStage() {
  glDeleteTextures(num_outputs_, textures_.data());
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
  glDeleteFramebuffers(1, &fbo_);
  glDeleteProgram(program_);
}

void FragmentStage::Clear(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
  glDrawBuffers(num_outputs_, buffers_.data());
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FragmentStage::Resize(int width, int height) {
  for (auto it = textures_.begin(); it != textures_.end(); it++) {
    glBindTexture(GL_TEXTURE_RECTANGLE, *it);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, format(), width, height, 0,
                 GL_RGBA, GL_FLOAT, nullptr);
  }
}

void FragmentStage::Draw() {
  glUseProgram(program_);
  glBindVertexArray(vao_);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
  glDrawBuffers(num_outputs_, buffers_.data());
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

bool FragmentStage::BuildShaderProgram(GLuint& out_program, const char* fs_source) {
  GLuint program = glCreateProgram();
  GLint compiled;

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &VS_SOURCE, nullptr);
  glCompileShader(vs);

  glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    std::cerr << "[fs] failed to compile vertex shader!" << std::endl;
    return false;
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fs_source, nullptr);
  glCompileShader(fs);

  glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    std::cerr << "[fs] failed to compile fragment shader!" << std::endl;
    return false;
  }

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  // TODO: check link status

  glDetachShader(program, vs);
  glDetachShader(program, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);

  out_program = program;
  return true;
}

}  // namespace pipe
}  // namespace quarke
