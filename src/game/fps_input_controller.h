#ifndef QUARKE_SRC_GAME_FPS_INPUT_CONTROLLER_H_
#define QUARKE_SRC_GAME_FPS_INPUT_CONTROLLER_H_

#include "game/input_controller.h"

namespace quarke {
namespace game {

class Camera;

/**
 * An input controller translating input events to FPS-style movement commands.
 */
class FPSInputController : public InputController {
 public:
  /**
   * Movement mask.
   */
  enum Direction {
    NONE    = 0x0,
    FORWARD = 0x1,
    BACK    = 0x2,
    LEFT    = 0x4,
    RIGHT   = 0x8
  };

  /**
   * Creates a new FPSInputController manipulating the given camera.
   */
  FPSInputController(Camera& camera) : camera_(camera), direction_(NONE) {}

  bool OnKeyEvent(int keycode, int action, int modifiers) override;
  bool OnMouseButton(int button, int action, int modifiers) override;
  bool OnMouseMove(double x, double y) override;

  Direction direction() { return direction_; }

 private:
  Camera& camera_;
  Direction direction_;
};

}  // namespace game
}  // namespace quarke
#endif  // QUARKE_SRC_GAME_FPS_INPUT_CONTROLLER_H_
