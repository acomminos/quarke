#include "game/scene.h"
#include "mat/solid_material.h"
#include "mat/textured_material.h"
#include "util/toytga.h"
#include <glm/gtc/matrix_transform.hpp>

namespace quarke {
namespace game {

Scene::Scene(int width, int height)
  : camera_(width, height)
  , rot(0) {
  // XXX: testing
  //meshes_.push_back(geo::Mesh::FromOBJ("model/teapot.obj"));
  auto armadillo = geo::Mesh::FromOBJ("model/armadillo.obj");
  armadillo->set_color(glm::vec4(0.2, 0.6, 0.2, 1.0));
  meshes_.push_back(std::move(armadillo));

  auto terrain = geo::Mesh::FromOBJ("model/terrain.obj");
  terrain->set_color(glm::vec4(0.1, 0.1, 0.1, 1.0));
  terrain->set_transform(
      glm::translate(glm::mat4(), glm::vec3(0.0, -1.0, 0.0)) *
      glm::scale(terrain->transform(), glm::vec3(200.0, 1.0, 200.0)));
  meshes_.push_back(std::move(terrain));

  auto bunny = geo::Mesh::FromOBJ("model/bunny.obj");
  bunny->set_color(glm::vec4(0.8, 0.8, 0.8, 1.0));
  bunny->set_transform(
      glm::translate(glm::mat4(), glm::vec3(1.5, -1.0, 0.0)) *
      glm::rotate(glm::mat4(), 180.f, glm::vec3(0.0, 1.0, 0.0)) *
      glm::scale(glm::mat4(), glm::vec3(0.25, 0.25, 0.25))
  );
  meshes_.push_back(std::move(bunny));

  auto wall = geo::Mesh::FromOBJ("model/wall.obj");
  wall->set_transform(
      glm::translate(glm::mat4(), glm::vec3(0.0, 1.5, 2.5)) *
      glm::scale(glm::mat4(), glm::vec3(3.0, 3.0, 3.0)));
  textured_meshes_.push_back(std::move(wall));

  util::TGA::Descriptor pepper;
  if (util::TGA::LoadTGA("tex/ad.tga", pepper)) {
    // XXX: assume power of two texture size and rgb.
    assert(pepper.format == util::TGA::Descriptor::TGA_RGB24);
    GLuint pepper_tex;
    glGenTextures(1, &pepper_tex);
    glBindTexture(GL_TEXTURE_2D, pepper_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pepper.width, pepper.height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, pepper.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    pepper_tex_ = pepper_tex;
    free(pepper.data);
  }
}

void Scene::Update(float dt) {
  // XXX: demo
  const float rot_speed = 1.2; // rotational speed in radians
  const float rot_dist = 1.0;
  const float base_z = -5.0;
  const float base_y = 2.0;
  rot = rot + (dt * rot_speed);
  float x = rot_dist * cos(rot);
  float z = rot_dist * sin(rot);

  camera_.LookAt({ x, base_y, z + base_z }, { 0.0, 1.0, 0.0 }, { 0.0, 1.0, 0.0 });
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
  mat::SolidMaterial solidMaterial;
  StubMaterialIterator iter(&solidMaterial, meshes_);
  geom_->Render(camera_, iter);

  // FIXME: demo.
  mat::TexturedMaterial texturedMaterial(GL_TEXTURE_2D, pepper_tex_);
  StubMaterialIterator texIter(&texturedMaterial, textured_meshes_);
  geom_->Render(camera_, texIter);

  pipe::PointLight light1 = {1.0f, 8.0f, glm::vec3(0, 4.f, 1.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)};
  pipe::PointLight light2 = {1.0f, 4.0f, glm::vec3(0.f, 2.f, -2.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)};
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

  // Draw light buffer in top left
  glBindFramebuffer(GL_READ_FRAMEBUFFER, lighting_->fbo());
  glReadBuffer(lighting_->buffer());
  glBlitFramebuffer(0, 0, width, height,
                    0, height/3, 2 * width / 3, height,
                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  // TODO: draw light buffer in top right, draw blended buffer in top left
}

void Scene::OnResize(int width, int height) {
  camera_.SetViewport(width, height);
  if (lighting_)
    lighting_->Resize(width, height);
}

}  // namespace game
}  // namespace quarke
