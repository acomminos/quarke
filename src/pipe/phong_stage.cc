#include "pipe/phong_stage.h"
#include "game/camera.h"
#include <iostream>

namespace quarke {
namespace pipe {

// a triangle strip covering ndc
static const GLfloat SCREEN_VERTICES[] = {
  // (x, y, u, v)
  1.0, 1.0, 1.0, 1.0,
  -1.0, 1.0, 0.0, 1.0,
  -1.0, -1.0, 0.0, 0.0,
  1.0, -1.0, 1.0, 0.0,
};

static const char* PHONG_POINT_VS = R"(
#version 330

in vec2 position;
in vec2 uv;

out vec2 vUV;

void main(void) {
  vUV = uv;
  gl_Position = vec4(position, 0.0, 1.0);
}
)";

static const char* PHONG_POINT_FS = R"(
#version 330

uniform sampler2D colorSampler;
uniform sampler2D normalSampler;
uniform sampler2D positionSampler;

uniform vec3 eyePosition;
uniform vec3 lightPosition;
uniform vec4 lightColor;

in vec2 vUV;

out vec4 outLight;

void main(void) {
  vec4 albedo = texture(colorSampler, vUV);
  vec3 normal = texture(normalSampler, vUV).xyz;
  vec3 pos = texture(positionSampler, vUV).xyz;

  vec3 d = normalize(lightPosition - pos);
  float dIntensity = dot(d, normal);
  vec4 diffuseColor = albedo * lightColor * dIntensity;

  // TODO: specular component.

  outLight = diffuseColor;
}
)";

std::unique_ptr<PhongStage> PhongStage::Create(int width,
                                               int height,
                                               GLuint color_tex,
                                               GLuint normal_tex,
                                               GLuint position_tex) {
  const GLenum LIGHT_BUFFER = GL_COLOR_ATTACHMENT0;

  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  GLuint light_tex;
  glGenTextures(1, &light_tex);
  glBindTexture(GL_TEXTURE_RECTANGLE, light_tex);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, light_format(), width, height, 0,
               GL_RGBA, GL_FLOAT, nullptr);
  glFramebufferTexture(GL_FRAMEBUFFER, LIGHT_BUFFER, color_tex, 0);

  GLuint depth_tex;
  glGenTextures(1, &depth_tex);
  glBindTexture(GL_TEXTURE_RECTANGLE, depth_tex);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "[phong] Incomplete framebuffer." << std::endl;
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &light_tex);
    glDeleteTextures(1, &depth_tex);
    return nullptr;
  }

  GLuint screen_vbo;
  glGenBuffers(1, &screen_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(SCREEN_VERTICES), SCREEN_VERTICES, GL_STATIC_DRAW);

  GLuint program, vs, fs;
  if (!BuildShaderProgram(program, vs, fs)) {
    std::cerr << "[phong] Building shader program failed." << std::endl;
    return nullptr;
  }

  return std::make_unique<PhongStage>(width, height, fbo, LIGHT_BUFFER,
                                      light_tex, screen_vbo, color_tex,
                                      normal_tex, position_tex, depth_tex);
}

PhongStage::PhongStage(int width, int height, GLuint light_fbo,
                       GLuint light_buffer, GLuint light_tex,
                       GLuint screen_vbo, GLuint color_tex, GLuint normal_tex,
                       GLuint position_tex, GLuint depth_tex)
  : out_width_(width), out_height_(height)
  , light_fbo_(light_fbo), light_buffer_(light_buffer), light_tex_(light_tex)
  , screen_vbo_(screen_vbo), color_tex_(color_tex), normal_tex_(normal_tex)
  , position_tex_(position_tex), depth_tex_(depth_tex) {}

void PhongStage::Clear() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fbo_);
  glDrawBuffers(1, (const GLenum*) &light_buffer_);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void PhongStage::Illuminate(const game::Camera& camera, const PointLight& light) {
  glDrawBuffers(1, (const GLenum*) &light_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, screen_vbo_);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

bool PhongStage::BuildShaderProgram(GLuint& out_program, GLuint& out_vs, GLuint& out_fs) {
  GLuint program = glCreateProgram();

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLint vs_len = sizeof(PHONG_POINT_VS);
  glShaderSource(vs, 1, &PHONG_POINT_VS, nullptr);
  glCompileShader(vs);

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  GLint fs_len = sizeof(PHONG_POINT_FS);
  glShaderSource(fs, 1, &PHONG_POINT_FS, nullptr);
  glCompileShader(fs);

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  // TODO: get errors here

  out_program = program;
  out_vs = vs;
  out_fs = fs;
  return true;
}

}  // namespace pipe
}  // namespace quarke
