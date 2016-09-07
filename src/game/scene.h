#ifndef QUARKE_SRC_GAME_SCENE_H_
#define QUARKE_SRC_GAME_SCENE_H_

namespace quarke {
namespace game {

// A scene manages the rendering world state (such as the projection matrix),
// as well as the gfx pipeline.
class Scene {
 public:
  void Render();
  void Resize(int width, int height);
 private:
  // TODO
  //glm::mat4 world_matrix_;
  //glm::vec3 position_;
  //float time_elapsed_;
};

}  // namespace game
}  // namespace quarke

#endif  // QUARKE_SRC_GAME_SCENE_H_
