#pragma once

#include "lve_descriptors.hpp"
#include "lve_texture.hpp"

namespace lve {

// The pool owns the descriptor allocation and must outlive this material.
// Texture and descriptor contents remain immutable while draws are in flight.
class LveMaterial {
 public:
  LveMaterial(LveDescriptorSetLayout &layout, LveDescriptorPool &pool,
              std::shared_ptr<LveTexture> texture);
  LveMaterial(const LveMaterial &) = delete;
  LveMaterial &operator=(const LveMaterial &) = delete;

  VkDescriptorSet getDescriptorSet() const { return descriptorSet; }

 private:
  std::shared_ptr<LveTexture> texture;
  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};

}  // namespace lve
