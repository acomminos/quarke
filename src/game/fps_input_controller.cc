#include "game/fps_input_controller.h"
#include <GLFW/glfw3.h>

namespace quarke {
namespace game {

bool FPSInputController::OnKeyEvent(int keycode, int action, int modifiers) {
  // XXX: assumes maximum mask of 8 bits.
  int press_mask = keycode != GLFW_RELEASE ? 0xFF : 0;
  switch (keycode) {
    case GLFW_KEY_W:
      direction_ = static_cast<Direction>((direction_ & ~FORWARD) |
                                          (FORWARD & press_mask));
      return true;
    case GLFW_KEY_A:
      direction_ = static_cast<Direction>((direction_ & ~LEFT) |
                                          (LEFT & press_mask));
      return true;
    case GLFW_KEY_S:
      direction_ = static_cast<Direction>((direction_ & ~BACK) |
                                          (BACK & press_mask));
      return true;
    case GLFW_KEY_D:
      direction_ = static_cast<Direction>((direction_ & ~RIGHT) |
                                          (RIGHT & press_mask));
      return true;
  }
  return false;
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

}  // namespace game
}  // namespace quarke
