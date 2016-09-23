#include "pipe/phong_stage.h"
#include "game/camera.h"

namespace quarke {
namespace pipe {

// a triangle strip covering ndc
static const GLfloat SCREEN_VERTICES[] = {
  1.0, 1.0,
  -1.0, 1.0,
  -1.0, -1.0,
  1.0, -1.0,
};

static const char* PHONG_POINT_VS = R"(
#version 330

in vec2 position;

void main(void) {
  gl_Position = position;
}
)";

static const char* PHONG_POINT_FS = R"(
#version 330

uniform sampler2D color;
uniform sampler2D light;
uniform sampler2D depth;

uniform vec3 lightPosition;
uniform vec3 lightColor;

out vec4 outLight;

void main(void) {
  outLight = vec4(1.0, 1.0, 1.0, 1.0);
}
)";

std::unique_ptr<PhongStage> PhongStage::Create(GLuint color_tex,
                                               GLuint normal_tex,
                                               GLuint depth_tex) {
  GLuint screen_vbo;
  glGenBuffers(1, &screen_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(SCREEN_VERTICES), SCREEN_VERTICES, GL_STATIC_DRAW);
  return std::make_unique<PhongStage>(0, 0, 0, screen_vbo, color_tex, normal_tex, depth_tex);
}

PhongStage::PhongStage(GLuint light_fbo, GLuint light_buffer, GLuint light_tex,
                       GLuint screen_vbo, GLuint color_tex, GLuint normal_tex,
                       GLuint depth_tex)
  : light_fbo_(light_fbo), light_buffer_(light_buffer), light_tex_(light_tex)
  , screen_vbo_(screen_vbo), color_tex_(color_tex), normal_tex_(normal_tex)
  , depth_tex_(depth_tex) {}

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

}  // namespace pipe
}  // namespace quarke
