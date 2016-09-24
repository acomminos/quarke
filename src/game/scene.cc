#include "game/scene.h"
#include "mat/solid_material.h"
#include <glm/gtc/matrix_transform.hpp>

namespace quarke {
namespace game {

Scene::Scene(int width, int height)
  : camera_(width, height)
  , rot(0) {
  // XXX: testing
  //meshes_.push_back(geo::Mesh::FromOBJ("model/teapot.obj"));
  meshes_.push_back(geo::Mesh::FromOBJ("model/armadillo.obj"));

  auto terrain = geo::Mesh::FromOBJ("model/terrain.obj");
  terrain->set_transform(
      glm::translate(glm::mat4(), glm::vec3(0.0, -1.0, 0.0)) *
      glm::scale(terrain->transform(), glm::vec3(50.0, 1.0, 50.0)));
  meshes_.push_back(std::move(terrain));
}

void Scene::Update(float dt) {
  // XXX: demo
  const float rot_speed = 1.5; // rotational speed in radians
  const float rot_dist = 5.0;
  const float rot_y = 3.0;
  rot = rot + (dt * rot_speed);
  float x = rot_dist * cos(rot);
  float z = rot_dist * sin(rot);

  camera_.LookAt({ x, rot_y, z }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 });
}

void Scene::Render() {
  if (!geom_) {
    // TODO: instantiate this elsewhere where we can handle failures.
    //       in addition, make the mesh interface somewhat exposed.
    geom_ = pipe::GeometryStage::Create(camera_.viewport_width(),
                                        camera_.viewport_height());
    assert(geom_);
  }

  if (!lighting_) {
    lighting_ = pipe::PhongStage::Create(camera_.viewport_width(),
                                         camera_.viewport_height(),
                                         geom_->color_tex(),
                                         geom_->normal_tex(),
                                         geom_->position_tex());
    assert(lighting_);
  }

  geom_->Clear();
  // FIXME: render with basic material for now.
  mat::SolidMaterial basicMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0));
  StubMaterialIterator iter(&basicMaterial, meshes_);
  geom_->Render(camera_, iter);

  pipe::PointLight light1 = {1.0f, 10.0f, glm::vec3(0, 3.f, 0), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)};
  pipe::PointLight light2 = {1.0f, 20.0f, glm::vec3(-5.f, 0.f, 0), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)};
  lighting_->Clear();
  lighting_->Illuminate(camera_, light1);
  //lighting_->Illuminate(camera_, light2);

  // FIXME: this blit is the worst
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, geom_->fbo());

  int width = camera_.viewport_width();
  int height = camera_.viewport_height();

  // Draw color buffer in bottom left
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(0, 0, width, height,
                    0, 0, width/3, height/3,
                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  // Draw normal buffer in bottom middle
  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glBlitFramebuffer(0, 0, width, height,
                    width/3, 0, 2 * width / 3, height/3,
                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  // Draw position buffer in bottom right
  glReadBuffer(GL_COLOR_ATTACHMENT2);
  glBlitFramebuffer(0, 0, width, height,
                    2 * width/3, 0, width, height/3,
                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  // Draw light buffer in middle right
  glBindFramebuffer(GL_READ_FRAMEBUFFER, lighting_->fbo());
  glReadBuffer(lighting_->buffer());
  glBlitFramebuffer(0, 0, width, height,
                    2 * width/3, height/3, width, 2 * height/3,
                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  // TODO: draw light buffer in top right, draw blended buffer in top left
}

void Scene::OnResize(int width, int height) {
  camera_.SetViewport(width, height);
}

}  // namespace game
}  // namespace quarke
