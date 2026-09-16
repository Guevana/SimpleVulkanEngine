#include "lve_material.hpp"

#include <stdexcept>
#include <utility>

namespace lve {
LveMaterial::LveMaterial(LveDescriptorSetLayout &layout, LveDescriptorPool &pool,
                         std::shared_ptr<LveTexture> texture)
    : texture{std::move(texture)} {
  if (!this->texture) throw std::invalid_argument("material requires a texture");
  auto imageInfo = this->texture->descriptorInfo();
  if (!LveDescriptorWriter(layout, pool).writeImage(0, &imageInfo).build(descriptorSet)) {
    throw std::runtime_error("failed to allocate material descriptor set");
  }
}
}  // namespace lve
