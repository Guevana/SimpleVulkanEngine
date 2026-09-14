#include "lve_FirstApp.hpp"

#include <stdexcept>
#include <array>

namespace lve {
    FirstAPP::FirstAPP() {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    FirstAPP::~FirstAPP() {
        vkDeviceWaitIdle(lveDevice.device());
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void FirstAPP::run() {
        while (!lveWindow.shouldClose())
        {
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(lveDevice.device());
    }


    void FirstAPP::loadModels() {
        std::vector<LveModel::Vertex> vertices{
        // 第一个三角形
        {{-0.8f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-0.2f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},

        // 第二个三角形
        {{0.2f, -0.5f}, {1.0f, 1.0f, 0.0f}},
        {{0.8f, -0.5f}, {0.0f, 1.0f, 1.0f}},
        {{0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}}};
        lveModel = std::make_unique<LveModel>(lveDevice, vertices);
    }

    void FirstAPP::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("PipelineLayout创建失败");
        }
    }

     void FirstAPP::createPipeline() {
        assert(lveSwapChain != nullptr && "交换链未创建");
        assert(pipelineLayout != nullptr && "管道布局未创建");

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = lveSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;

        lvePipeline = std::make_unique<LvePipeline>(
            lveDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig);
     }

     void FirstAPP::recreateSwapChain()
     {
        auto extent = lveWindow.getExtent();  
        while (extent.width == 0 || extent.height == 0)
        {
            extent =lveWindow.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(lveDevice.device());

        if(lveSwapChain == nullptr) {
        lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
        } else {
            lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, std::move(lveSwapChain));
            if (lveSwapChain->imageCount() != commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
        createPipeline();
     }
     
     void FirstAPP::createCommandBuffers() {
        commandBuffers.resize(lveSwapChain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = lveDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("分配命令缓冲区失败");
        }

        for (int i = 0; i<commandBuffers.size(); i++) {
           
        }
    }

    void FirstAPP::freeCommandBuffers()
    {
        vkFreeCommandBuffers(
            lveDevice.device(), 
            lveDevice.getCommandPool(), 
            static_cast<uint32_t>(commandBuffers.size()), 
            commandBuffers.data());
        commandBuffers.clear();
    }

    void FirstAPP::recordCommandBuffer(int imageIndex) {
         VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("开始记录命令缓冲区失败");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = lveSwapChain->getRenderPass();
            renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
            viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
            vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
            vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);


            lvePipeline->bind(commandBuffers[imageIndex]);
            lveModel->bind(commandBuffers[imageIndex]);
            lveModel->draw(commandBuffers[imageIndex]);

            vkCmdEndRenderPass(commandBuffers[imageIndex]);
            if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
                throw std::runtime_error("录制缓冲指令失败");
            }
    }

     void FirstAPP::drawFrame() {
        uint32_t imageIndex = 0;
        auto result = lveSwapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS) {
            throw std::runtime_error("无法获取交换链纹理, VkResult=" + std::to_string(result));
        }

        recordCommandBuffer(imageIndex);
        result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
            lveWindow.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS) {
            throw std::runtime_error("无法呈现交换链纹理");
        }
     }
}
