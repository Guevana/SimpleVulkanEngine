#pragma once

#include "lve_camera.hpp"
#include "lve_device.hpp"
#include "lve_GameObject.hpp"
#include "lve_Pipeline.hpp"
#include "lve_frame_info.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class SimpleRenderSystem {
 public:
  SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout,
                     VkDescriptorSetLayout materialSetLayout, std::shared_ptr<LveMaterial> defaultMaterial);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  void renderGameObjects(FrameInfo &frameInfo, std::vector<LveGameObject> &gameObjects);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout materialSetLayout);
  void createPipeline(VkRenderPass renderPass);

  LveDevice &lveDevice;

  std::unique_ptr<LvePipeline> lvePipeline;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  std::shared_ptr<LveMaterial> defaultMaterial;
};
}  // namespace lve
