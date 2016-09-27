#include "pipe/omni_shadow_stage.h"
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "geo/mesh.h"

namespace quarke {
namespace pipe {

static const float Z_NEAR = 0.1f;
static const float Z_FAR = 1000.f;
static const char* VS_SOURCE = R"(
#version 330 core

uniform mat4 mvp_matrix;

// FIXME: this assumes position 0, should build from stream and consts instead.
layout(location = 0) in vec3 position;

void main() {
  gl_Position = mvp_matrix * vec4(position, 1.0);
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
  glCompileShader(vs);
  glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    std::cerr << "[oss] failed to compile vertex shader!" << std::endl;
    return nullptr;
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &FS_SOURCE, nullptr);
  glCompileShader(fs);
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
  for (int i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, depth_internal_format(),
                 texture_size, texture_size, 0, depth_format(), GL_FLOAT, nullptr);
  }


  return std::make_unique<OmniShadowStage>(program, fbo, cube_texture,
                                           texture_size);
}

OmniShadowStage::OmniShadowStage(GLuint program, GLuint fbo,
                                 GLuint cube_texture, GLsizei texture_size)
  : program_(program), fbo_(fbo)
  , cube_texture_(cube_texture), texture_size_(texture_size) {
  uniform_transform_ = glGetUniformLocation(program, "mvp_matrix");
}

void OmniShadowStage::BuildShadowMap(const glm::vec3 position,
                                     MaterialIterator& iter) {
  glUseProgram(program_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cube_texture_);
  for (int i = 0; i < 6; i++) {
    GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
    RenderFace(face, position, iter);
    iter.Reset();
  }
}

void OmniShadowStage::RenderFace(GLenum face, const glm::vec3 position,
                                 MaterialIterator& iter) {
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
  glUniformMatrix4fv(uniform_transform_, 1, GL_FALSE, glm::value_ptr(transform));
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, cube_texture_, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glClear(GL_DEPTH_BUFFER_BIT);

  while (auto matit = iter.NextMaterial()) {
    while (auto mit = matit->Next()) {
      geo::VertexBuffer& buffer = mit->array_buffer();
      glBindVertexArray(buffer.vertex_array());
      glDrawArrays(GL_TRIANGLES, 0, mit->num_vertices());
    }
  }

}

}  // namespace pipe
}  // namespace quarke
