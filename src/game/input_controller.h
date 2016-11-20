#ifndef QUARKE_SRC_GAME_INPUT_CONTROLLER_H_
#define QUARKE_SRC_GAME_INPUT_CONTROLLER_H_

namespace quarke {
namespace game {

class InputController {
 public:
  /**
   * Called when a key event is ready to be dispatched to the input controller.
   * @return true if the input event should not propagate to ancestors.
   */
  virtual bool OnKeyEvent(int keycode, int action, int modifiers) = 0;
  /**
   * Called when a mouse button event is ready to be dispatched to the input
   * controller.
   * @return true if the input event should not propagate to ancestors.
   */
  virtual bool OnMouseButton(int button, int action, int modifiers) = 0;
  /**
   * Called when a mouse move event is ready to be dispatched to the input
   * controller.
   * @return true if the input event should not propagate to ancestors.
   */
  virtual bool OnMouseMove(double x, double y) = 0;
};

}  // namespace game
}  // namespace quarke

#endif  // QUARKE_SRC_GAME_INPUT_CONTROLLER_H_
