#include "game/scene.h"
#include "mat/solid_material.h"
#include "mat/textured_material.h"
#include "util/toytga.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace quarke {
namespace game {

Scene::Scene(int width, int height)
  : camera_(width, height)
  , rot(0) {

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
  meshes_.AddMesh(solid_material_.get(), std::move(armadillo));

  auto terrain = geo::Mesh::FromOBJ("model/terrain.obj");
  terrain->set_color(glm::vec4(0.8, 0.8, 0.8, 1.0));
  terrain->set_transform(
      glm::translate(glm::mat4(), glm::vec3(0.0, -1.0, 0.0)) *
      glm::scale(terrain->transform(), glm::vec3(200.0, 1.0, 200.0)));
  meshes_.AddMesh(solid_material_.get(), std::move(terrain));

  auto bunny = geo::Mesh::FromOBJ("model/bunny.obj");
  bunny->set_color(glm::vec4(0.8, 0.8, 0.8, 1.0));
  bunny->set_transform(
      glm::translate(glm::mat4(), glm::vec3(1.5, -1.0, 0.0)) *
      glm::rotate(glm::mat4(), 180.f, glm::vec3(0.0, 1.0, 0.0)) *
      glm::scale(glm::mat4(), glm::vec3(0.25, 0.25, 0.25))
  );
  meshes_.AddMesh(solid_material_.get(), std::move(bunny));

  auto wall = geo::Mesh::FromOBJ("model/wall.obj");
  wall->set_transform(
      glm::translate(glm::mat4(), glm::vec3(0.0, 1.5, 2.5)) *
      glm::scale(glm::mat4(), glm::vec3(3.0, 3.0, 3.0)) *
      glm::rotate(glm::mat4(), glm::pi<float>(), glm::vec3(0.f, 1.f, 0.f)));
  meshes_.AddMesh(textured_material_.get(), std::move(wall));

  point_lights_.push_back({1.0f, 15.0f, glm::vec3(0.f, 5.f, 0.f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)});
}

void Scene::Update(float dt) {
  // XXX: demo
  const float rot_speed = 1.2; // rotational speed in radians
  const float rot_dist = 5.0;
  const float base_z = -10.0;
  const float base_y = 5.0;
  rot = rot + (dt * rot_speed);
  float x = rot_dist * cos(rot);
  float z = rot_dist * sin(rot);

  camera_.LookAt({ x, base_y, z + base_z }, { 0.0, 2.0, 0.0 }, { 0.0, 1.0, 0.0 });
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
    const GLsizei TEXTURE_RESOLUTION = 1024;
    omni_shadow_ = pipe::OmniShadowStage::Create(TEXTURE_RESOLUTION);
    assert(omni_shadow_);
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
    omni_shadow_->BuildShadowMap(it->position, mesh_iter);
    lighting_->Illuminate(camera_, *it, omni_shadow_->cube_texture());
  }

  // FIXME: this blit is the worst
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, geom_->fbo());

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

  // Draw ambient buffer in middle right
  glBindFramebuffer(GL_READ_FRAMEBUFFER, ambient_->ambient_fbo());
  glReadBuffer(ambient_->ambient_buffer());
  glBlitFramebuffer(0, 0, width, height,
                    2 * width / 3, height/3, width, 2 * height / 3,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);

  // Draw light buffer in top left
  glBindFramebuffer(GL_READ_FRAMEBUFFER, lighting_->fbo());
  glReadBuffer(lighting_->buffer());
  glBlitFramebuffer(0, 0, width, height,
                    0, height/3, 2 * width / 3, height,
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);

  // TODO: draw light buffer in top right, draw blended buffer in top left
}

void Scene::OnResize(int width, int height) {
  camera_.SetViewport(width, height);
  if (lighting_)
    lighting_->Resize(width, height);
}

}  // namespace game
}  // namespace quarke
