#pragma once

#include "lve_renderer.hpp"

namespace lve {
class LveImguiLayer {
 public:
  LveImguiLayer(LveWindow& window, LveDevice& device, const SwapChainInfo& info);
  ~LveImguiLayer();
  LveImguiLayer(const LveImguiLayer&) = delete;
  LveImguiLayer& operator=(const LveImguiLayer&) = delete;

  void beginFrame();
  void render(VkCommandBuffer commandBuffer);
  void syncSwapChain(const SwapChainInfo& info);

 private:
  void initVulkan(const SwapChainInfo& info);
  void shutdown() noexcept;
  LveDevice& device;
  SwapChainInfo swapChainInfo{};
  bool contextCreated{false};
};
}  // namespace lve
