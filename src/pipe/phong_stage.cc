#include "pipe/phong_stage.h"
#include "game/camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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

static const char* PHONG_POINT_VS = R"(
#version 330 core

layout(location = 0) in vec2 position;

void main(void) {
  gl_Position = vec4(position, 0.0, 1.0);
}
)";

static const char* PHONG_POINT_FS = R"(
#version 330 core

uniform sampler2DRect colorSampler;
uniform sampler2DRect normalSampler;
uniform sampler2DRect positionSampler;
uniform sampler2DRect depthSampler;

uniform vec3 eye;
uniform vec3 lightPosition;
uniform vec4 lightColor;
uniform float lightDistance;

// omni-directional cube map containing fragment distances from the light source
uniform samplerCube shadowSamplerCube;

// TODO: implement this per-material. perhaps a specular buffer?
const float specularPower = 5.0;

// amount of depth buffer variance that's acceptable to reject within
const float shadowMapEpsilon = 0.05;

out vec4 outLight;

void main(void) {
  vec4 albedo = texture(colorSampler, gl_FragCoord.xy);
  vec3 normal = texture(normalSampler, gl_FragCoord.xy).xyz;
  vec3 pos = texture(positionSampler, gl_FragCoord.xy).xyz;
  //float depth = texture(depthSampler, gl_FragCoord.xy).r;

  vec3 pos_light = pos - lightPosition;
  float distSq = dot(pos_light, pos_light);

  // shadow mapping
  float shadowDepth = texture(shadowSamplerCube, pos_light).r;
  float shadowIntensity = 0.25; // soft shadows
  if (abs(shadowDepth - distSq) < shadowMapEpsilon) {
    shadowIntensity = 1.0; // if within a margin of error of the correct depth, illuminate
  }

  vec3 d = normalize(lightPosition - pos);
  float dIntensity = max(dot(d, normal), 0.0);
  vec4 diffuseColor = albedo * lightColor * dIntensity;

  vec3 dEye = normalize(eye - pos);
  vec3 half = normalize(dEye + d);

  float sIntensity = max(dot(half, normal), 0.0);
  vec4 specularColor = lightColor * pow(sIntensity, specularPower);

  // Use a nonlinearity for falloff around the edges.
  float dist = sqrt(distSq);
  float lightIntensity = max(1.0 - pow(dist/lightDistance, 2.0), 0.0);

  outLight = shadowIntensity * lightIntensity * (diffuseColor + specularColor);
}
)";

std::unique_ptr<PhongStage> PhongStage::Create(int width,
                                               int height,
                                               GLuint color_tex,
                                               GLuint normal_tex,
                                               GLuint position_tex,
                                               GLuint depth_tex) {
  const GLenum LIGHT_BUFFER = GL_COLOR_ATTACHMENT0;

  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  GLuint light_tex;
  glGenTextures(1, &light_tex);
  glBindTexture(GL_TEXTURE_RECTANGLE, light_tex);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, format(), width, height, 0,
               GL_RGBA, GL_FLOAT, nullptr);
  glFramebufferTexture(GL_FRAMEBUFFER, LIGHT_BUFFER, light_tex, 0);

  GLuint light_depth_tex;
  glGenTextures(1, &light_depth_tex);
  glBindTexture(GL_TEXTURE_RECTANGLE, light_depth_tex);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light_depth_tex, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "[phong] Incomplete framebuffer." << std::endl;
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &light_tex);
    glDeleteTextures(1, &light_depth_tex);
    return nullptr;
  }

  GLuint program;
  if (!BuildShaderProgram(program)) {
    std::cerr << "[phong] Building shader program failed." << std::endl;
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
  // TODO: restore current VAO? may be called within context.

  return std::make_unique<PhongStage>(width, height, program, fbo,
                                      LIGHT_BUFFER, light_tex, light_depth_tex,
                                      screen_vbo, screen_vao, color_tex, normal_tex,
                                      position_tex, depth_tex);
}
PhongStage::PhongStage(int width, int height, GLuint program,
                       GLuint light_fbo, GLuint light_buffer,
                       GLuint light_tex, GLuint light_depth_tex,
                       GLuint screen_vbo, GLuint screen_vao,
                       GLuint color_tex, GLuint normal_tex, GLuint position_tex,
                       GLuint depth_tex)
  : out_width_(width), out_height_(height), program_(program)
  , light_fbo_(light_fbo), light_buffer_(light_buffer), light_tex_(light_tex)
  , light_depth_tex_(light_depth_tex), screen_vbo_(screen_vbo)
  , screen_vao_(screen_vao), color_tex_(color_tex), normal_tex_(normal_tex)
  , position_tex_(position_tex), depth_tex_(depth_tex)
{
  // TODO: construct vs/fs using streams+consts so we don't have to have magic strings
  color_sampler_location_ = glGetUniformLocation(program, "colorSampler");
  normal_sampler_location_ = glGetUniformLocation(program, "normalSampler");
  position_sampler_location_ = glGetUniformLocation(program, "positionSampler");
  depth_sampler_location_ = glGetUniformLocation(program, "depthSampler");
  eye_position_location_ = glGetUniformLocation(program, "eye");
  light_position_location_ = glGetUniformLocation(program, "lightPosition");
  light_color_location_ = glGetUniformLocation(program, "lightColor");
  light_distance_location_ = glGetUniformLocation(program, "lightDistance");
  shadow_sampler_location_ = glGetUniformLocation(program, "shadowSamplerCube");
}

void PhongStage::Clear() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fbo_);
  glDrawBuffers(1, (const GLenum*) &light_buffer_);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void PhongStage::Illuminate(const game::Camera& camera, const PointLight& light,
                            GLuint shadow_cubemap) {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fbo_);
  glDrawBuffers(1, (const GLenum*) &light_buffer_);

  glUseProgram(program_);

  glBindVertexArray(screen_vao_);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_RECTANGLE, color_tex_);
  glUniform1i(color_sampler_location_, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_RECTANGLE, normal_tex_);
  glUniform1i(normal_sampler_location_, 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_RECTANGLE, position_tex_);
  glUniform1i(position_sampler_location_, 2);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, depth_tex_);
  glUniform1i(depth_sampler_location_, 3);

  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_cubemap);
  glUniform1i(shadow_sampler_location_, 4);

  glUniform3fv(eye_position_location_, 1, glm::value_ptr(camera.Position()));
  glUniform3fv(light_position_location_, 1, glm::value_ptr(light.position));
  glUniform4fv(light_color_location_, 1, glm::value_ptr(light.color));
  glUniform1f(light_distance_location_, light.max_distance);

  // Additive blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  glDisable(GL_DEPTH_TEST);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void PhongStage::Resize(int width, int height) {
  out_width_ = width;
  out_height_ = height;
  glBindTexture(GL_TEXTURE_RECTANGLE, light_tex_);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, format(), width, height, 0,
               GL_RGBA, GL_FLOAT, nullptr);
  glBindTexture(GL_TEXTURE_RECTANGLE, light_depth_tex_);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
}

bool PhongStage::BuildShaderProgram(GLuint& out_program) {
  GLuint program = glCreateProgram();
  GLint compiled;

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLint vs_len = sizeof(PHONG_POINT_VS);
  glShaderSource(vs, 1, &PHONG_POINT_VS, nullptr);
  glCompileShader(vs);

  glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    std::cerr << "[phong] failed to compile vertex shader!" << std::endl;
    return false;
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  GLint fs_len = sizeof(PHONG_POINT_FS);
  glShaderSource(fs, 1, &PHONG_POINT_FS, nullptr);
  glCompileShader(fs);

  glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    std::cerr << "[phong] failed to compile fragment shader!" << std::endl;
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
