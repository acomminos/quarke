#include "game/scene.h"
#include "game/game.h"
#include "mat/solid_material.h"
#include "mat/textured_material.h"
#include "util/toytga.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace quarke {
namespace game {

Scene::Scene(const Game& game, int width, int height)
  : game_(game)
  , camera_(width, height)
  , manual_control_(false)
  , rot(0)
  , active_stage_(COMPOSITE) {

  solid_material_ = std::make_unique<mat::SolidMaterial>();

  util::TGA::Descriptor pepper;
  assert(util::TGA::LoadTGA("tex/ad.tga", pepper));
  // XXX: assume power of two texture size and rgb.
  assert(pepper.format == util::TGA::Descriptor::TGA_RGB24);
  GLuint pepper_tex;
  glGenTextures(1, &pepper_tex);
  glBindTexture(GL_TEXTURE_2D, pepper_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pepper.width, pepper.height, 0,
      GL_RGB, GL_UNSIGNED_BYTE, pepper.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
  glGenerateMipmap(GL_TEXTURE_2D);
  pepper_tex_ = pepper_tex;
  free(pepper.data);
  textured_material_ = std::make_unique<mat::TexturedMaterial>(GL_TEXTURE_2D, pepper_tex_);

  // XXX: Load some demo data.
  auto armadillo = geo::Mesh::FromOBJ("model/armadillo.obj");
  armadillo->set_color(glm::vec4(0.2, 0.6, 0.2, 1.0));
  armadillo->set_transform(glm::translate(glm::mat4(), glm::vec3(0.f, 1.f, 0.f)));
  meshes_.AddMesh(solid_material_.get(), std::move(armadillo));

  auto terrain = geo::Mesh::FromOBJ("model/huge_box.obj");
  terrain->set_color(glm::vec4(0.8, 0.8, 0.8, 1.0));
  terrain->set_transform(
      glm::translate(glm::mat4(), glm::vec3(0.f, 0.7f, 0.f)) *
      glm::scale(glm::mat4(), glm::vec3(4.0, 4.0, 4.0)));
  meshes_.AddMesh(solid_material_.get(), std::move(terrain));

  auto bunny = geo::Mesh::FromOBJ("model/bunny.obj");
  bunny->set_color(glm::vec4(1.0, 1.0, 1.0, 1.0));
  bunny->set_transform(
      glm::translate(glm::mat4(), glm::vec3(2.5, 0.0, 0.0)) *
      glm::rotate(glm::mat4(), 180.f, glm::vec3(0.0, 1.0, 0.0)) *
      glm::scale(glm::mat4(), glm::vec3(0.25, 0.25, 0.25))
  );
  meshes_.AddMesh(solid_material_.get(), std::move(bunny));

  auto wall = geo::Mesh::FromOBJ("model/wall.obj");
  wall->set_transform(
      glm::translate(glm::mat4(), glm::vec3(0.0, 2.5, 2.5)) *
      glm::scale(glm::mat4(), glm::vec3(3.0, 3.0, 3.0)) *
      glm::rotate(glm::mat4(), glm::pi<float>(), glm::vec3(0.f, 1.f, 0.f)));
  meshes_.AddMesh(textured_material_.get(), std::move(wall));

  point_lights_.push_back({1.0f, 30.0f, glm::vec3(0.f, 7.f, -5.f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)});
}

void Scene::Update(float dt) {
  GLFWwindow* window = game_.window();

  // TODO: migrate this to a demo input controller.
  const float MANUAL_TRANSLATE_SPEED = 5.f * dt; // in world units/s
  const float MANUAL_ROTATION_SPEED = glm::pi<float>()/3.f * dt; // in radians/s
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    manual_control_ = true;
    position_ += eye_ * MANUAL_TRANSLATE_SPEED;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    manual_control_ = true;
    position_ += eye_ * -MANUAL_TRANSLATE_SPEED;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    manual_control_ = true;
    position_ += -glm::cross(eye_, glm::vec3(0, 1, 0)) * MANUAL_TRANSLATE_SPEED;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    manual_control_ = true;
    position_ += glm::cross(eye_, glm::vec3(0, 1, 0)) * MANUAL_TRANSLATE_SPEED;
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    manual_control_ = true;
    eye_ = glm::rotate(eye_, MANUAL_ROTATION_SPEED, glm::cross(eye_, glm::vec3(0, 1, 0)));
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    manual_control_ = true;
    eye_ = glm::rotate(eye_, -MANUAL_ROTATION_SPEED, glm::cross(eye_, glm::vec3(0, 1, 0)));
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    manual_control_ = true;
    eye_ = glm::rotate(eye_, MANUAL_ROTATION_SPEED, glm::vec3(0, 1, 0));
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    manual_control_ = true;
    eye_ = glm::rotate(eye_, -MANUAL_ROTATION_SPEED, glm::vec3(0, 1, 0));
  }

  eye_ = glm::normalize(eye_);

  // XXX: demo
  if (!manual_control_) {
    const float rot_speed = 1.2; // rotational speed in radians
    const float rot_dist = 3.0;
    const float base_z = -8.0;
    const float base_y = 4.0;
    const glm::vec3 target(0.0, 2.0, 0.0);
    rot = rot + (dt * rot_speed);
    float x = rot_dist * cos(rot);
    float z = rot_dist * sin(rot);
    position_ = glm::vec3(x, base_y, base_z + z);
    eye_ = target - position_;
  }

  camera_.LookAt(position_, position_ + eye_, { 0.0, 1.0, 0.0 });
}

void Scene::Render() {
  if (!geom_) {
    // TODO: instantiate this elsewhere where we can handle failures.
    //       in addition, make the mesh interface somewhat exposed.
    geom_ = pipe::GeometryStage::Create(camera_.viewport_width(),
                                        camera_.viewport_height());
    assert(geom_);
  }

  if (!ambient_) {
    ambient_ = pipe::AmbientStage::Create(camera_.viewport_width(),
                                          camera_.viewport_height(),
                                          glm::vec4(0.2, 0.2, 0.2, 1.0));
    assert(ambient_);
  }

  if (!lighting_) {
    lighting_ = pipe::PhongStage::Create(camera_.viewport_width(),
                                         camera_.viewport_height(),
                                         geom_->color_tex(),
                                         geom_->normal_tex(),
                                         geom_->position_tex(),
                                         geom_->depth_tex());
    assert(lighting_);
  }

  if (!omni_shadow_) {
    const GLsizei TEXTURE_RESOLUTION = 2048;
    omni_shadow_ = pipe::OmniShadowStage::Create(TEXTURE_RESOLUTION);
    assert(omni_shadow_);
  }

  if (!ssao_) {
    ssao_ = pipe::SSAOStage::Create(camera_.viewport_width(),
                                    camera_.viewport_height());
    assert(ssao_);
  }

  auto mesh_iter = meshes_.Iterator();
  geom_->Clear();
  geom_->Render(camera_, mesh_iter);

  ambient_->Clear();
  ambient_->Render(geom_->color_tex());

  lighting_->Clear();

  int width = camera_.viewport_width();
  int height = camera_.viewport_height();

  // XXX: share a light buffer between the ambient and phong stages.
  glBindFramebuffer(GL_READ_FRAMEBUFFER, ambient_->ambient_fbo());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lighting_->fbo());
  glReadBuffer(ambient_->ambient_buffer());
  const GLenum dbuffers[] = { lighting_->buffer() };
  glDrawBuffers(1, dbuffers);
  glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

  mesh_iter.Reset();
  for (auto it = point_lights_.begin(); it != point_lights_.end(); it++) {
    omni_shadow_->BuildShadowMap(camera_, it->position, mesh_iter);
    lighting_->Illuminate(camera_, *it, omni_shadow_->cube_texture());
  }

  ssao_->Clear();
  ssao_->Render(camera_, lighting_->tex(), geom_->depth_tex());

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  switch (active_stage_) {
    case COMPOSITE:
      // TODO: have an actual composite output. for now, just blit SSAO.
      glBindFramebuffer(GL_READ_FRAMEBUFFER, lighting_->fbo());
      glReadBuffer(lighting_->buffer());
      break;
    case ALBEDO:
      glBindFramebuffer(GL_READ_FRAMEBUFFER, geom_->fbo());
      glReadBuffer(geom_->color_buffer());
      break;
    case NORMAL:
      glBindFramebuffer(GL_READ_FRAMEBUFFER, geom_->fbo());
      glReadBuffer(geom_->normal_buffer());
      break;
    case POSITION:
      glBindFramebuffer(GL_READ_FRAMEBUFFER, geom_->fbo());
      glReadBuffer(geom_->position_buffer());
      break;
    case AMBIENT:
      glBindFramebuffer(GL_READ_FRAMEBUFFER, ambient_->ambient_fbo());
      glReadBuffer(ambient_->ambient_buffer());
      break;
  }

  glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void Scene::OnResize(int width, int height) {
  camera_.SetViewport(width, height);
  if (lighting_)
    lighting_->Resize(width, height);
}

void Scene::OnKeyEvent(int key, int scancode, int action, int mods) {
  // TODO: one-off key event handling.
  //       for movement, use the main loop.
  if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9 &&
      (key - GLFW_KEY_1 < NUM_STAGES)) {
    // Stage selection shortcut
    active_stage_ = (enum ActiveStage) (key - GLFW_KEY_1);
  }
}

}  // namespace game
}  // namespace quarke
