#pragma once

#include "lve_device.hpp"

#include <cstdint>
#include <memory>
#include <span>
#include <string>

namespace lve {

class LveTexture {
 public:
  enum class ColorSpace { Srgb, Linear };
  struct Config {
    ColorSpace colorSpace = ColorSpace::Srgb;
    VkFilter filter = VK_FILTER_LINEAR;
    VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  };

  static std::unique_ptr<LveTexture> createTextureFromFile(
      LveDevice &device, const std::string &filepath, Config config);
  static std::unique_ptr<LveTexture> createTextureFromFile(
      LveDevice &device, const std::string &filepath) {
    return createTextureFromFile(device, filepath, Config{});
  }
  static std::unique_ptr<LveTexture> createTextureFromRgba(
      LveDevice &device, uint32_t width, uint32_t height,
      std::span<const uint8_t> pixels, Config config);
  static std::unique_ptr<LveTexture> createTextureFromRgba(
      LveDevice &device, uint32_t width, uint32_t height, std::span<const uint8_t> pixels) {
    return createTextureFromRgba(device, width, height, pixels, Config{});
  }

  ~LveTexture();
  LveTexture(const LveTexture &) = delete;
  LveTexture &operator=(const LveTexture &) = delete;

  VkDescriptorImageInfo descriptorInfo() const;
  uint32_t getWidth() const { return width; }
  uint32_t getHeight() const { return height; }
  VkFormat getFormat() const { return format; }

 private:
  explicit LveTexture(LveDevice &device) : lveDevice{device} {}
  void upload(uint32_t imageWidth, uint32_t imageHeight,
              std::span<const uint8_t> pixels, Config config);
  void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

  LveDevice &lveDevice;
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkImageView imageView = VK_NULL_HANDLE;
  VkSampler sampler = VK_NULL_HANDLE;
  uint32_t width = 0;
  uint32_t height = 0;
  VkFormat format = VK_FORMAT_UNDEFINED;
};

}  // namespace lve
