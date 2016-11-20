#ifndef QUARKE_SRC_GAME_SCENE_H_
#define QUARKE_SRC_GAME_SCENE_H_

#include <map>
#include <memory>
#include <list>
#include "game/camera.h"
#include "geo/mesh.h"
#include "mat/solid_material.h"
#include "mat/textured_material.h"
#include "pipe/ambient_stage.h"
#include "pipe/geometry_stage.h"
#include "pipe/phong_stage.h"
#include "pipe/omni_shadow_stage.h"
#include "pipe/ssao_stage.h"
#include "geo/linked_mesh_collection.h"

namespace quarke {
namespace game {

class Game;

// A scene manages the rendering world state (such as the projection matrix),
// as well as the gfx pipeline.
class Scene {
 public:
  Scene(const Game& game, int width, int height);

  void Update(float dt);
  void Render();

  // Called when the engine has resized the scene.
  // The dimensions provided are in device pixel units.
  void OnResize(int width, int height);

  // Maps directly to a GLFW key callback.
  // Called immediately after swapbuffers.
  void OnKeyEvent(int key, int scancode, int action, int mods);

 private:
  const Game& game_;
  Camera camera_;
  bool manual_control_; // true if the user has pressed a camera key.
  glm::vec3 eye_; // normalized eye-target viewing vector
  glm::vec3 position_; // position of camera
  float rot; // tmp
  GLuint pepper_tex_; // tmp
  std::vector<pipe::PointLight> point_lights_;

  // TODO: should we put the pipeline here?
  //       or move into separate pipeline class?
  std::unique_ptr<pipe::GeometryStage> geom_;
  std::unique_ptr<pipe::AmbientStage> ambient_;
  std::unique_ptr<pipe::PhongStage> lighting_;
  std::unique_ptr<pipe::OmniShadowStage> omni_shadow_;
  std::unique_ptr<pipe::SSAOStage> ssao_;

  // The primary stage to display.
  // Each option corresponds to an offset from GLFW_KEY_1.
  enum ActiveStage {
    COMPOSITE = 0,
    ALBEDO,
    NORMAL,
    POSITION,
    AMBIENT,
    NUM_STAGES
  } active_stage_;

  geo::LinkedMeshCollection meshes_;

  // TODO: move these to a global material cache.
  std::unique_ptr<mat::SolidMaterial> solid_material_;
  std::unique_ptr<mat::TexturedMaterial> textured_material_;
};

}  // namespace game
}  // namespace quarke

#endif  // QUARKE_SRC_GAME_SCENE_H_
