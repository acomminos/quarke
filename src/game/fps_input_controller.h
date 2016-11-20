#ifndef QUARKE_SRC_GAME_FPS_INPUT_CONTROLLER_H_
#define QUARKE_SRC_GAME_FPS_INPUT_CONTROLLER_H_

#include "game/input_controller.h"
#include <cstring>
#include <queue>

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
    FORWARD = 0,
    BACK,
    LEFT,
    RIGHT,
    NUM_DIRECTIONS
  };

  struct MovementEvent {
    double time_elapsed[NUM_DIRECTIONS];
  };

  static constexpr double NO_TIME = -1;

  /**
   * Creates a new FPSInputController manipulating the given camera.
   */
  FPSInputController(Camera& camera) : camera_(camera) {
    memset(total_times, NO_TIME, sizeof(total_times));
    memset(key_start_times, NO_TIME, sizeof(key_start_times));
  }

  bool OnKeyEvent(int keycode, int action, int modifiers) override;
  bool OnMouseButton(int button, int action, int modifiers) override;
  bool OnMouseMove(double x, double y) override;

  MovementEvent ComputeMovementDeltas();

  Direction direction() { return direction_; }

 private:
  double ClaimDeltaTime(Direction dir);

  Camera& camera_;
  Direction direction_;
  /**
   * The total recorded time for elapsed key up+down sequences since the last
   * request for deltas.
   */
  double total_times[NUM_DIRECTIONS];

  /**
   * For in-progress key down events, the delta epoch.
   */
  double key_start_times[NUM_DIRECTIONS];
};

}  // namespace game
}  // namespace quarke
#endif  // QUARKE_SRC_GAME_FPS_INPUT_CONTROLLER_H_
