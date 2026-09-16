#include "keyboard_movement_controller.hpp"

// std
#include <limits>

namespace lve {

void KeyboardMovementController::moveInPlaneXZ(
    GLFWwindow* window, float dt, LveGameObject& gameObject,
    bool mouseEnabled, bool keyboardEnabled) {
  const auto pressed = [window, keyboardEnabled](int key) {
    return keyboardEnabled && glfwGetKey(window, key) == GLFW_PRESS;
  };
  glm::vec3 rotate{0};
  if (pressed(keys.lookRight)) rotate.y += 1.f;
  if (pressed(keys.lookLeft)) rotate.y -= 1.f;
  if (pressed(keys.lookUp)) rotate.x += 1.f;
  if (pressed(keys.lookDown)) rotate.x -= 1.f;

  double mouseX;
  double mouseY;
  glfwGetCursorPos(window, &mouseX, &mouseY);

  if (firstMouse) {
    lastMouseX = mouseX;
    lastMouseY = mouseY;
    firstMouse = false;
  }

  const float mouseDeltaX = static_cast<float>(mouseX - lastMouseX);
  const float mouseDeltaY = static_cast<float>(lastMouseY - mouseY);
  lastMouseX = mouseX;
  lastMouseY = mouseY;

  if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
  }
  if (mouseEnabled) {
    gameObject.transform.rotation.y += mouseSensitivity * mouseDeltaX;
    gameObject.transform.rotation.x += mouseSensitivity * mouseDeltaY;
  } else {
    resetMouse();
  }

  // limit pitch values between about +/- 85ish degrees
  gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
  gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

  float yaw = gameObject.transform.rotation.y;
  const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
  const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
  const glm::vec3 upDir{0.f, -1.f, 0.f};

  glm::vec3 moveDir{0.f};
  if (pressed(keys.moveForward)) moveDir += forwardDir;
  if (pressed(keys.moveBackward)) moveDir -= forwardDir;
  if (pressed(keys.moveRight)) moveDir += rightDir;
  if (pressed(keys.moveLeft)) moveDir -= rightDir;
  if (pressed(keys.moveUp)) moveDir += upDir;
  if (pressed(keys.moveDown)) moveDir -= upDir;

  if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
  }
}
}  // namespace lve
