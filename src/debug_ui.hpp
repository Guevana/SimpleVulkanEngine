#pragma once

#include "keyboard_movement_controller.hpp"
#include "lve_renderer.hpp"

namespace lve {
class DebugUI {
 public:
  void draw(std::vector<LveGameObject>& objects, LveGameObject& viewer,
            KeyboardMovementController& controller, LveRenderer& renderer,
            float frameTime);
  float fieldOfView() const { return fovDegrees; }

 private:
  LveGameObject::id_t selectedObject{0};
  float fovDegrees{50.f};
  bool showDemo{false};
};
}  // namespace lve
