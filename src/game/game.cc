#include "game/game.h"

// XXX: temporary includes for testing
#include "game/scene.h"
#include "geo/mesh.h"

namespace quarke {
namespace game {

static Game* current_game;

/* static */
int Game::Run(int* argc, char** argv[]) {
  assert(!current_game);
  if (!glfwInit())
    return -1;

  // TODO: fetch window size from args?
  GLFWwindow* window = glfwCreateWindow(640, 480, "quarke", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  Game game(window);
  current_game = &game;
  game.Loop();

  current_game = nullptr;
  glfwTerminate();
  return 0;
}

void Game::Loop() {
  glfwMakeContextCurrent(window_);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

  int width, height;
  glfwGetFramebufferSize(window_, &width, &height);
  Scene scene(width, height); // XXX: TEMP

  glfwSetFramebufferSizeCallback(window_, [](GLFWwindow*, int width, int height) {
  });

  while (!glfwWindowShouldClose(window_)) {
    scene.Render();
    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
}


}  // namespace game
}  // namespace quarke
