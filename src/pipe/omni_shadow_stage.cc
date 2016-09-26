#include "pipe/omni_shadow_stage.h"
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace quarke {
namespace pipe {

static const float Z_NEAR = 0.1f;
static const float Z_FAR = 1000.f;
static const char* VS_SOURCE = R"(
#version 330 core

uniform mat4 transform;
layout(location = 0) in vec3 position;

void main() {
  gl_Position = transform * position;
}
)";
static const char* FS_SOURCE = R"(
#version 330 core

void main() {
  // FIXME: should we transform our depth data?
}
)";


std::unique_ptr<OmniShadowStage> OmniShadowStage::Create(GLsizei texture_size) {
  GLuint fbo;
  glGenFramebuffers(1, &fbo);

  GLuint program = glCreateProgram();

  GLint compiled;
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &VS_SOURCE, nullptr);
  glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    std::cerr << "[oss] failed to compile vertex shader!" << std::endl;
    return nullptr;
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &FS_SOURCE, nullptr);
  glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    std::cerr << "[oss] failed to compile fragment shader!" << std::endl;
    return nullptr;
  }

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  GLint linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (!linked) {
    std::cerr << "[oss] failed to link program!" << std::endl;
    return nullptr;
  }

  glDetachShader(program, vs);
  glDetachShader(program, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);

  GLuint cube_texture;
  glGenTextures(1, &cube_texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cube_texture);

  GLuint textures[6];
  glGenTextures(6, textures);
  for (int i = 0; i < sizeof(textures) / sizeof(textures[0]); i++) {
    glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, textures[i]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, depth_internal_format(),
                 texture_size, texture_size, 0, depth_format(), GL_FLOAT, nullptr);
  }


  return std::make_unique<OmniShadowStage>(program, fbo, textures, cube_texture,
                                           texture_size);
}

OmniShadowStage::OmniShadowStage(GLuint program, GLuint fbo, GLuint textures[6],
                                 GLuint cube_texture, GLsizei texture_size)
  : program_(program), fbo_(fbo)
  , cube_texture_(cube_texture), texture_size_(texture_size) {
  std::memcpy(textures_, textures, sizeof(GLuint) * 6);
  uniform_transform_ = glGetUniformLocation(program, "transform");
}

void OmniShadowStage::BuildShadowMap(const glm::vec3 position) {
  for (int i = 0; i < 6; i++) {
    GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
    //textures_[i] = ...
  }
}

void OmniShadowStage::RenderFace(GLenum face, const glm::vec3 position) {
  glm::vec3 dir;
  switch (face) {
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      dir = glm::vec3(1.f, 0.f, 0.f);
      break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      dir = glm::vec3(-1.f, 0.f, 0.f);
      break;
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      dir = glm::vec3(0.f, 1.f, 0.f);
      break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      dir = glm::vec3(0.f, -1.f, 0.f);
      break;
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      dir = glm::vec3(0.f, 0.f, 1.f);
      break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      dir = glm::vec3(0.f, 0.f, -1.f);
      break;
    default:
      assert(true);
      break;
  }

  glm::mat4 transform = glm::perspective(90.f, 1.f, Z_NEAR, Z_FAR) *
                        glm::lookAt(position, position + dir, glm::vec3(0.f, 1.f, 0.f));

}

}  // namespace pipe
}  // namespace quarke
