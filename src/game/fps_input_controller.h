#ifndef QUARKE_SRC_GAME_FPS_INPUT_CONTROLLER_H_
#define QUARKE_SRC_GAME_FPS_INPUT_CONTROLLER_H_

#include "game/input_controller.h"
#include <cstring>
#include <queue>

namespace quarke {
namespace game {

enum Direction {
  FORWARD = 0,
  BACK,
  LEFT,
  RIGHT,
  NUM_DIRECTIONS
};

/**
 * An input controller translating input events to FPS-style movement commands.
 */
class FPSInputController : public InputController {
 public:

  struct MovementEvent {
    float time_elapsed[NUM_DIRECTIONS];
  };

  static constexpr double NO_START_TIME = -1;

  FPSInputController() {
    memset(total_times, 0, sizeof(total_times));
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
      key_start_times[i] = NO_START_TIME;
    }
  }

  bool OnKeyEvent(int keycode, int action, int modifiers) override;
  bool OnMouseButton(int button, int action, int modifiers) override;
  bool OnMouseMove(double x, double y) override;

  /**
   * Stores movement deltas since the last call to ComputeMovementDeltas in the
   * provided event, returning true if any events have occurred.
   */
  bool ComputeMovementDeltas(MovementEvent& event);

 private:
  double ClaimDeltaTime(Direction dir, bool release);

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
