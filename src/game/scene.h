#ifndef QUARKE_SRC_GAME_SCENE_H_
#define QUARKE_SRC_GAME_SCENE_H_

#include <glm/glm.hpp>

namespace quarke {
namespace game {

// A scene manages the rendering world state (such as the projection matrix),
// as well as the gfx pipeline.
class Scene {
 public:
  Scene();

  void Render();

  // Called when the engine has resized the scene.
  void OnResize(int width, int height);

  int viewport_width() const { return viewport_width_; }
  int viewport_height() const { return viewport_height_; }

 private:
  int viewport_width_;
  int viewport_height_;

  const float fov_; // degrees
  const float near_;
  const float far_;
  glm::mat4 projection_;

  // TODO: should we put the pipeline here?

  // TODO
  //glm::mat4 world_matrix_;
  //glm::vec3 position_;
  //float time_elapsed_;
};

}  // namespace game
}  // namespace quarke

#endif  // QUARKE_SRC_GAME_SCENE_H_
