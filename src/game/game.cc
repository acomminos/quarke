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

#ifdef QUARKE_DEBUG
  // FIXME: possibly disable this, even in debug builds. maybe flag?
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // QUARKE_DEBUG
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

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
