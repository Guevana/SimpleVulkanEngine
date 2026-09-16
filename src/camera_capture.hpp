#pragma once

namespace lve {
// Input ownership is latched on the right-button press, not on hover changes.
// This is independent of GLFW/ImGui so cancellation paths can be tested.
class CameraCapture {
 public:
  bool update(bool rightDown, bool focused, bool escape, bool uiWantsMouse) {
    if (!focused || !rightDown || escape) release();
    if (focused && !escape && rightDown && !previousRightDown && !uiWantsMouse) {
      captured = true;
      ++version;
    }
    previousRightDown = rightDown;
    return captured;
  }
  void release() {
    if (captured) {
      captured = false;
      ++version;
    }
  }
  void cancel() {
    release();
    previousRightDown = true;
  }
  void buttonReleased() {
    release();
    previousRightDown = false;
  }
  bool active() const { return captured; }
  unsigned int revision() const { return version; }

 private:
  bool captured{false};
  bool previousRightDown{false};
  unsigned int version{0};
};
}  // namespace lve
