#include "lve_FirstApp.hpp"

#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <array>

namespace lve {
    FirstAPP::FirstAPP() {loadGameObjects();}

    FirstAPP::~FirstAPP() {}

    void FirstAPP::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        while (!lveWindow.shouldClose())
        {
            glfwPollEvents();

            if (auto commandBuffer = lveRenderer.beginFrame()) {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }

        }
        
        vkDeviceWaitIdle(lveDevice.device());
}


    void FirstAPP::loadGameObjects() {
        std::vector<LveModel::Vertex> vertices{
        // 第一个三角形
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},};
        auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);

        auto triangle = LveGameObject::createLveGameObject();
        triangle.model = lveModel;
        triangle.color = {0.1f, 0.8f, 0.1f};
        triangle.translation2d.translation.x = 0.5f;
        triangle.translation2d.scale.x = 1.0f;
        triangle.translation2d.rotation = 0.3f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    }

}
