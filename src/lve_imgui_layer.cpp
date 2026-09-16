#include "lve_imgui_layer.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <cstdio>
#include <stdexcept>

static_assert(IMGUI_VERSION_NUM >= 19291, "ImGui 1.92.9b or newer is required");

namespace lve {
namespace {
void checkVkResult(VkResult result) {
  if (result != VK_SUCCESS) {
    std::fprintf(stderr, "ImGui Vulkan result: %d\n", static_cast<int>(result));
  }
  if (result < 0) throw std::runtime_error("ImGui Vulkan operation failed");
}
}  // namespace

LveImguiLayer::LveImguiLayer(LveWindow& window, LveDevice& deviceRef,
                           const SwapChainInfo& info) : device{deviceRef} {
  IMGUI_CHECKVERSION();
  try {
    ImGui::CreateContext();
    contextCreated = true;
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    if (!ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true)) {
      throw std::runtime_error("Failed to initialize ImGui GLFW backend");
    }
    initVulkan(info);
  } catch (...) {
    shutdown();
    throw;
  }
}

LveImguiLayer::~LveImguiLayer() { shutdown(); }

void LveImguiLayer::shutdown() noexcept {
  if (!contextCreated) return;
  vkDeviceWaitIdle(device.device());
  // Backend user data also covers partially completed initialization.
  try {
    if (ImGui::GetIO().BackendRendererUserData) ImGui_ImplVulkan_Shutdown();
  } catch (const std::exception& error) {
    std::fprintf(stderr, "ImGui shutdown: %s\n", error.what());
  }
  if (ImGui::GetIO().BackendPlatformUserData) ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  contextCreated = false;
}

void LveImguiLayer::initVulkan(const SwapChainInfo& info) {
  if (info.minImageCount < 2 || info.imageCount < info.minImageCount) {
    throw std::runtime_error("Invalid ImGui swapchain image counts");
  }
  ImGui_ImplVulkan_InitInfo init{};
  init.ApiVersion = device.apiVersion();
  init.Instance = device.getInstance();
  init.PhysicalDevice = device.getPhysicalDevice();
  init.Device = device.device();
  init.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily;
  init.Queue = device.graphicsQueue();
  init.DescriptorPoolSize = 64;
  init.MinImageCount = info.minImageCount;
  init.ImageCount = info.imageCount;
  init.PipelineInfoMain.RenderPass = info.renderPass;
  init.PipelineInfoMain.Subpass = 0;
  init.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init.CheckVkResultFn = checkVkResult;
  if (!ImGui_ImplVulkan_Init(&init)) {
    throw std::runtime_error("Failed to initialize ImGui Vulkan backend");
  }
  swapChainInfo = info;
}

void LveImguiLayer::syncSwapChain(const SwapChainInfo& info) {
  if (info.generation == swapChainInfo.generation) return;
  if (info.minImageCount != swapChainInfo.minImageCount ||
      info.imageCount != swapChainInfo.imageCount) {
    checkVkResult(vkDeviceWaitIdle(device.device()));
    ImGui_ImplVulkan_Shutdown();
    initVulkan(info);
  }
  // Renderer rejects format changes; otherwise its render passes are compatible.
  swapChainInfo = info;
}

void LveImguiLayer::beginFrame() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void LveImguiLayer::render(VkCommandBuffer commandBuffer) {
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
}  // namespace lve
