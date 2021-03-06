#include "game/fps_input_controller.h"
#include <GLFW/glfw3.h>

namespace quarke {
namespace game {

bool FPSInputController::OnKeyEvent(int keycode, int action, int modifiers) {
  Direction dir;
  switch (keycode) {
    case GLFW_KEY_W:
      dir = FORWARD;
      break;
    case GLFW_KEY_A:
      dir = LEFT;
      break;
    case GLFW_KEY_S:
      dir = BACK;
      break;
    case GLFW_KEY_D:
      dir = RIGHT;
      break;
    default:
      return false;
  }

  switch (action) {
    case GLFW_PRESS:
      key_start_times[dir] = glfwGetTime();
      break;
    case GLFW_RELEASE:
      total_times[dir] += ClaimDeltaTime(dir, true);
      break;
  }

  return true;
}

bool FPSInputController::OnMouseButton(int button, int action, int modifiers) {
  switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      return true;
    case GLFW_MOUSE_BUTTON_RIGHT:
      return true;
  }
  return false;
}

bool FPSInputController::OnMouseMove(double x, double y) {
  return false;
}

bool FPSInputController::ComputeMovementDeltas(FPSInputController::MovementEvent& event) {
  bool has_changes = false;
  for (int i = 0; i < NUM_DIRECTIONS; i++) {
    Direction dir = static_cast<Direction>(i);
    event.time_elapsed[i] = total_times[i] + ClaimDeltaTime(dir, false);
    has_changes |= event.time_elapsed[i] > 0;
  }
  // Clear deltas after claiming them.
  memset(total_times, 0, sizeof(total_times));
  return has_changes;
}

double FPSInputController::ClaimDeltaTime(Direction dir, bool release) {
  double start = key_start_times[dir];
  if (start == NO_START_TIME) {
    // We received a key release without a corresponding press, ignore.
    return 0;
  }
  key_start_times[dir] = release ? NO_START_TIME : glfwGetTime();
  return glfwGetTime() - start;
}

}  // namespace game
}  // namespace quarke
