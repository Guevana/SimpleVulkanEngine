#include "debug_ui.hpp"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <string>

namespace lve {
namespace {
void editRotation(glm::vec3& rotation) {
  auto degrees = glm::degrees(rotation);
  if (ImGui::DragFloat3("Rotation (deg)", glm::value_ptr(degrees), 0.5f)) {
    rotation = glm::radians(degrees);
  }
}
}  // namespace

void DebugUI::draw(std::vector<LveGameObject>& objects, LveGameObject& viewer,
                   KeyboardMovementController& controller, LveRenderer& renderer,
                   float frameTime) {
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(310, 540), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Debug")) {
    if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
      ImGui::Text("Frame: %.3f ms", frameTime * 1000.f);
    }
    if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
      if (objects.empty()) {
        ImGui::TextUnformatted("No objects");
      } else {
        auto selected = std::find_if(objects.begin(), objects.end(),
            [this](const auto& object) { return object.id == selectedObject; });
        if (selected == objects.end()) {
          selected = objects.begin();
          selectedObject = selected->id;
        }
        const auto label = "Object " + std::to_string(selectedObject);
        if (ImGui::BeginCombo("Object", label.c_str())) {
          for (auto& object : objects) {
            const auto name = "Object " + std::to_string(object.id);
            if (ImGui::Selectable(name.c_str(), object.id == selectedObject)) {
              selectedObject = object.id;
            }
          }
          ImGui::EndCombo();
        }
        for (auto& object : objects) {
          if (object.id != selectedObject) continue;
          ImGui::PushID("ObjectTransform");
          ImGui::DragFloat3("Position", glm::value_ptr(object.transform.translation), 0.01f);
          editRotation(object.transform.rotation);
          ImGui::DragFloat3("Scale", glm::value_ptr(object.transform.scale), 0.01f,
                           0.001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
          object.transform.scale = glm::max(object.transform.scale, glm::vec3(0.001f));
          ImGui::PopID();
        }
      }
    }
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::PushID("Camera");
      ImGui::DragFloat3("Position", glm::value_ptr(viewer.transform.translation), 0.01f);
      editRotation(viewer.transform.rotation);
      ImGui::SliderFloat("FOV", &fovDegrees, 10.f, 120.f, "%.1f deg", ImGuiSliderFlags_AlwaysClamp);
      ImGui::DragFloat("Move speed", &controller.moveSpeed, 0.05f, 0.01f, 100.f,
                       "%.2f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::DragFloat("Sensitivity", &controller.mouseSensitivity, 0.0001f, 0.0001f, 0.05f,
                       "%.4f", ImGuiSliderFlags_AlwaysClamp);
      if (ImGui::Button("Reset camera")) {
        viewer.transform = {};
        fovDegrees = 50.f;
        controller.moveSpeed = 1.f;
        controller.mouseSensitivity = 0.0025f;
        controller.resetMouse();
      }
      ImGui::PopID();
    }
    if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
      auto color = renderer.getClearColor();
      if (ImGui::ColorEdit3("Background", color.data())) renderer.setClearColor(color);
      ImGui::Checkbox("Demo Window", &showDemo);
    }
    if (ImGui::CollapsingHeader("Help")) {
      ImGui::TextWrapped("Hold right mouse outside UI to look around. While held, use WASD to move, Q/E down/up, or arrow keys to look. Release right mouse or press Escape to return to UI.");
    }
  }
  ImGui::End();
  if (showDemo) {
    ImGui::SetNextWindowPos(ImVec2(340, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(440, 550), ImGuiCond_FirstUseEver);
    ImGui::ShowDemoWindow(&showDemo);
  }
}
}  // namespace lve
