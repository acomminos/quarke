#ifndef QUARKE_SRC_GAME_GAME_H_
#define QUARKE_SRC_GAME_GAME_H_

#include <GLFW/glfw3.h>

namespace quarke {
namespace game {

class Game {
 public:
  // Instantiates a game instance.
  // Called only once from the program's entry point.
  static int Run(int* argc, char** argv[]);

  void Loop();
 private:
  Game(GLFWwindow* window) : window_(window) {}

  GLFWwindow* window_;
};

}  // namespace game
}  // namespace quarke

#endif  // QUARKE_SRC_GAME_GAME_H_
