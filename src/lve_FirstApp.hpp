#pragma once

#include "lve_Window.hpp"
#include "lvePipeline.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_model.hpp"

#include <memory>
#include <vector>

namespace lve{
    struct SimplePushConstantData {
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
    };

    class FirstAPP{
        public:
        FirstAPP();
        ~FirstAPP();

        FirstAPP(const FirstAPP &) = delete;
        FirstAPP &operator=(const FirstAPP &) =delete;
        
        static constexpr int Width = 800;
        static constexpr int Height = 600;

        void run();

        private:
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);

        LveWindow lveWindow{Width, Height, "Hello Vulkan!"};
        LveDevice lveDevice{lveWindow};
        std::unique_ptr<LveSwapChain> lveSwapChain;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<LveModel> lveModel;
    };

}