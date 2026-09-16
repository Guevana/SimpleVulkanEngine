#pragma once

#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"

// std
#include <cassert>
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace lve {
struct SwapChainInfo {
  uint64_t generation;
  VkRenderPass renderPass;
  uint32_t minImageCount;
  uint32_t imageCount;
};

class LveRenderer {
 public:
  LveRenderer(LveWindow &window, LveDevice &device);
  ~LveRenderer();

  LveRenderer(const LveRenderer &) = delete;
  LveRenderer &operator=(const LveRenderer &) = delete;

  VkRenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }
  SwapChainInfo getSwapChainInfo() const {
    return {swapChainGeneration, lveSwapChain->getRenderPass(),
            lveSwapChain->minImageCount(), static_cast<uint32_t>(lveSwapChain->imageCount())};
  }
  const std::array<float, 4>& getClearColor() const { return clearColor; }
  void setClearColor(const std::array<float, 4>& color) { clearColor = color; }
  float getAspectRadio() const {return lveSwapChain->extentAspectRatio(); }

  float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); }
  bool isFrameInProgress() const { return isFrameStarted; }

  VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
    return commandBuffers[currentFrameIndex];
  }

  int getFrameIndex() const {
    assert(isFrameStarted && "Cannot get frame index when frame not in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

 private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  LveWindow &lveWindow;
  LveDevice &lveDevice;
  std::unique_ptr<LveSwapChain> lveSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex;
  int currentFrameIndex{0};
  bool isFrameStarted{false};
  uint64_t swapChainGeneration{0};
  std::array<float, 4> clearColor{0.01f, 0.01f, 0.01f, 1.f};
};
}  // namespace lve
