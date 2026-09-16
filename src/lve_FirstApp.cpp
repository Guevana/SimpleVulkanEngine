#include "lve_FirstApp.hpp"

#include <utility>

#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "simple_render_system.hpp"
#include "lve_imgui_layer.hpp"
#include "debug_ui.hpp"
#include "lve_device_idle_guard.hpp"
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
    FirstAPP::FirstAPP() {
        globalPool = LveDescriptorPool::Builder(lveDevice)
                        .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                        .build();
        loadGameObjects();
    }

    FirstAPP::~FirstAPP() { vkDeviceWaitIdle(lveDevice.device()); }

    void FirstAPP::run() {
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i= 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<LveBuffer>(
            lveDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            uboBuffers[i]->map();
        }
        auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
                                    .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                                    .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            if (!LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i])) {
                throw std::runtime_error("failed to allocate global descriptor set");
            }
        }



        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout(),
            materialSetLayout->getDescriptorSetLayout(), defaultMaterial};
        LveImguiLayer imguiLayer{lveWindow, lveDevice, lveRenderer.getSwapChainInfo()};
        DebugUI debugUI{};
        LveCamera camera{};

        auto viewerObject = LveGameObject::createLveGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();
        auto captureRevision = lveWindow.captureRevision();
        // Wait before frame resources unwind on any exit.
        LveDeviceIdleGuard idleGuard{lveDevice};
        

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            /* ==== 以下为渲染循环 ====*/
            auto commandBuffer = lveRenderer.beginFrame();
            if (!commandBuffer) continue;

            //update
            int frameIndex = lveRenderer.getFrameIndex();

            FrameInfo frameInfo {
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex]
            };

            imguiLayer.syncSwapChain(lveRenderer.getSwapChainInfo());
            imguiLayer.beginFrame();

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

            camera.setPerspectiveProjection(glm::radians(debugUI.fieldOfView()), aspect, 0.1f, 10.0f);

            GlobalUbo ubo{};
            ubo.projectionView = camera.getProjection() * camera.getView();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            //render
            lveRenderer.beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
            imguiLayer.render(commandBuffer);
            lveRenderer.endSwapChainRenderPass(commandBuffer);


            lveRenderer.endFrame();

            /*==== 结束渲染循环 ====*/
        }
        
        vkDeviceWaitIdle(lveDevice.device());
}

    void FirstAPP::loadGameObjects() {
        std::array<uint8_t, 4> white{255, 255, 255, 255};
        std::vector<std::shared_ptr<LveTexture>> textures;
        textures.push_back(LveTexture::createTextureFromRgba(lveDevice, 1, 1, white));
        textures.push_back(LveTexture::createTextureFromFile(lveDevice, "textures/uv_quadrants.png"));
        materialSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).build();
        const auto materialCount = static_cast<uint32_t>(textures.size());
        materialPool = LveDescriptorPool::Builder(lveDevice).setMaxSets(materialCount)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, materialCount).build();
        defaultMaterial = std::make_shared<LveMaterial>(*materialSetLayout, *materialPool, textures[0]);
        auto imageMaterial = std::make_shared<LveMaterial>(*materialSetLayout, *materialPool, textures[1]);

        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "models/T-Rex.obj");

        auto gameObject = LveGameObject::createLveGameObject();
        gameObject.model = lveModel;
        gameObject.transform.translation = {0.0f, 2.0f, 5.0f};
        gameObject.transform.scale = {0.01f, 0.01f, 0.01f};
        gameObject.transform.rotation = {glm::radians(180.0f), 0.0f, 0.0f};
        gameObjects.push_back(std::move(gameObject));

        
        LveModel::Builder quad;
        quad.vertices = {
            {{-0.5f, -0.5f, 0.f}, {1.f, 1.f, 1.f}, {0.f, 0.f, -1.f}, {0.f, 0.f}},
            {{ 0.5f, -0.5f, 0.f}, {1.f, 1.f, 1.f}, {0.f, 0.f, -1.f}, {1.f, 0.f}},
            {{ 0.5f,  0.5f, 0.f}, {1.f, 1.f, 1.f}, {0.f, 0.f, -1.f}, {1.f, 1.f}},
            {{-0.5f,  0.5f, 0.f}, {1.f, 1.f, 1.f}, {0.f, 0.f, -1.f}, {0.f, 1.f}}};
        quad.indices = {0, 1, 2, 2, 3, 0};
        auto panel = LveGameObject::createLveGameObject();
        panel.model = std::make_shared<LveModel>(lveDevice, quad);
        panel.material = imageMaterial;
        panel.transform.translation = {0.8f, -0.2f, 2.5f};
        panel.transform.scale = {0.65f, 0.65f, 0.65f};
        gameObjects.push_back(std::move(panel));

        for (auto &vertex : quad.vertices) vertex.uv *= 2.f;
        auto repeatedPanel = LveGameObject::createLveGameObject();
        repeatedPanel.model = std::make_shared<LveModel>(lveDevice, quad);
        repeatedPanel.material = imageMaterial;
        repeatedPanel.transform.translation = {0.8f, 0.55f, 2.5f};
        repeatedPanel.transform.scale = {0.65f, 0.65f, 0.65f};
        gameObjects.push_back(std::move(repeatedPanel));
        
    }

}
