#include "lve_FirstApp.hpp"

#include <utility>

#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "simple_render_system.hpp"
#include "lve_imgui_layer.hpp"
#include "debug_ui.hpp"
#include <imgui.h>
#include <algorithm>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <chrono>
#include <stdexcept>
#include <cassert>
#include <array>

namespace lve {
    FirstAPP::FirstAPP() {loadGameObjects();}

    FirstAPP::~FirstAPP() {}

    void FirstAPP::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        LveImguiLayer imguiLayer{lveWindow, lveDevice, lveRenderer.getSwapChainInfo()};
        DebugUI debugUI{};
        LveCamera camera{};

        auto viewerObject = LveGameObject::createLveGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();
        auto captureRevision = lveWindow.captureRevision();
        

        while (!lveWindow.shouldClose())
        {
            glfwPollEvents();

            auto commandBuffer = lveRenderer.beginFrame();
            if (!commandBuffer) continue;
            imguiLayer.syncSwapChain(lveRenderer.getSwapChainInfo());
            imguiLayer.beginFrame();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            debugUI.draw(gameObjects, viewerObject, cameraController, lveRenderer, frameTime);
            const auto& io = ImGui::GetIO();
            const bool captured = lveWindow.updateCameraCapture(io.WantCaptureMouse);
            if (captureRevision != lveWindow.captureRevision()) {
                cameraController.resetMouse();
                captureRevision = lveWindow.captureRevision();
            }
            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), std::min(frameTime, 0.1f),
                                          viewerObject, captured, captured && !io.WantCaptureKeyboard);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            
            float aspect = lveRenderer.getAspectRadio();

            //正交投影
            //camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);

            camera.setPerspectiveProjection(glm::radians(debugUI.fieldOfView()), aspect, 0.1f, 10.0f);

            {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                imguiLayer.render(commandBuffer);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }

        }
        
        vkDeviceWaitIdle(lveDevice.device());
}

    void FirstAPP::loadGameObjects() {
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");

        auto gameObject = LveGameObject::createLveGameObject();
        gameObject.model = lveModel;
        gameObject.transform.translation = {0.0f, 0.0f, 2.5f};
        gameObject.transform.scale = {0.5f, 0.5f, 0.5f};
        gameObjects.push_back(std::move(gameObject));
    }

}
