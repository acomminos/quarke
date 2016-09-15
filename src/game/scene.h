#ifndef QUARKE_SRC_GAME_SCENE_H_
#define QUARKE_SRC_GAME_SCENE_H_

#include <memory>
#include "game/camera.h"
#include "pipe/geometry_stage.h"

namespace quarke {
namespace game {

// A scene manages the rendering world state (such as the projection matrix),
// as well as the gfx pipeline.
class Scene {
 public:
  Scene(int width, int height);

  void Render();

  // Called when the engine has resized the scene.
  // The dimensions provided are in device pixel units.
  void OnResize(int width, int height);

 private:
  Camera camera_;

  // TODO: should we put the pipeline here?
  std::unique_ptr<pipe::GeometryStage> geom_;
};

}  // namespace game
}  // namespace quarke

#endif  // QUARKE_SRC_GAME_SCENE_H_
