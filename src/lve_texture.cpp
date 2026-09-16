#include "lve_texture.hpp"
#include "lve_buffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <limits>
#include <stdexcept>

namespace lve {
namespace {
size_t pixelBytes(LveDevice &device, uint32_t width, uint32_t height) {
  const auto limit = device.properties.limits.maxImageDimension2D;
  if (!width || !height || width > limit || height > limit ||
      static_cast<uint64_t>(width) * height > std::numeric_limits<size_t>::max() / 4) {
    throw std::runtime_error("invalid or unsupported texture dimensions");
  }
  return static_cast<size_t>(width) * height * 4;
}

void check(VkResult result, const char *stage) {
  if (result != VK_SUCCESS) {
    throw std::runtime_error(std::string{stage} + " failed (VkResult " + std::to_string(result) + ")");
  }
}
}  // namespace

std::unique_ptr<LveTexture> LveTexture::createTextureFromFile(
    LveDevice &device, const std::string &filepath, Config config) {
  try {
    int width = 0, height = 0, channels = 0;
    if (!stbi_info(filepath.c_str(), &width, &height, &channels)) {
      throw std::runtime_error(std::string{"read image header: "} + stbi_failure_reason());
    }
    pixelBytes(device, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> pixels{
        stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha), stbi_image_free};
    if (!pixels) {
      throw std::runtime_error(std::string{"decode image: "} + stbi_failure_reason());
    }
    return createTextureFromRgba(device, width, height,
        {pixels.get(), pixelBytes(device, width, height)}, config);
  } catch (const std::exception &error) {
    throw std::runtime_error("texture '" + filepath + "': " + error.what());
  }
}

std::unique_ptr<LveTexture> LveTexture::createTextureFromRgba(
    LveDevice &device, uint32_t width, uint32_t height,
    std::span<const uint8_t> pixels, Config config) {
  // Own the object before uploading so partial GPU allocations are cleaned on failure.
  auto texture = std::unique_ptr<LveTexture>{new LveTexture{device}};
  texture->upload(width, height, pixels, config);
  return texture;
}

void LveTexture::upload(uint32_t imageWidth, uint32_t imageHeight,
                        std::span<const uint8_t> pixels, Config config) {
  const auto size = pixelBytes(lveDevice, imageWidth, imageHeight);
  if (pixels.size() != size || !pixels.data()) {
    throw std::runtime_error("RGBA texture data must contain exactly width * height * 4 bytes");
  }
  if (config.filter != VK_FILTER_LINEAR && config.filter != VK_FILTER_NEAREST) {
    throw std::runtime_error("texture filter must be linear or nearest");
  }
  if (config.addressMode != VK_SAMPLER_ADDRESS_MODE_REPEAT &&
      config.addressMode != VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT &&
      config.addressMode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE &&
      config.addressMode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) {
    throw std::runtime_error("unsupported texture address mode");
  }
  width = imageWidth;
  height = imageHeight;
  format = config.colorSpace == ColorSpace::Srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
  VkFormatFeatureFlags features = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
  if (config.filter == VK_FILTER_LINEAR) features |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
  lveDevice.findSupportedFormat({format}, VK_IMAGE_TILING_OPTIMAL, features);
  VkImageFormatProperties imageProperties{};
  check(vkGetPhysicalDeviceImageFormatProperties(lveDevice.getPhysicalDevice(), format,
      VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0, &imageProperties),
      "query texture image support");
  if (width > imageProperties.maxExtent.width || height > imageProperties.maxExtent.height ||
      size > imageProperties.maxResourceSize) {
    throw std::runtime_error("texture exceeds image format limits");
  }

  LveBuffer staging{lveDevice, size, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
  check(staging.map(), "map texture staging buffer");
  staging.writeToBuffer(const_cast<uint8_t *>(pixels.data()), size);

  VkImageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.imageType = VK_IMAGE_TYPE_2D;
  info.extent = {width, height, 1};
  info.mipLevels = 1;
  info.arrayLayers = 1;
  info.format = format;
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  info.samples = VK_SAMPLE_COUNT_1_BIT;
  info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  lveDevice.createImageWithInfo(info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);
  transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  lveDevice.copyBufferToImage(staging.getBuffer(), image, width, height, 1);
  transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  check(vkCreateImageView(lveDevice.device(), &viewInfo, nullptr, &imageView), "create texture image view");

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = config.filter;
  samplerInfo.minFilter = config.filter;
  samplerInfo.addressModeU = config.addressMode;
  samplerInfo.addressModeV = config.addressMode;
  samplerInfo.addressModeW = config.addressMode;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  samplerInfo.maxAnisotropy = 1.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;
  check(vkCreateSampler(lveDevice.device(), &samplerInfo, nullptr, &sampler), "create texture sampler");
}

void LveTexture::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  VkPipelineStageFlags srcStage, dstStage;
  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::runtime_error("unsupported texture layout transition");
  }
  auto command = lveDevice.beginSingleTimeCommands();
  vkCmdPipelineBarrier(command, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
  lveDevice.endSingleTimeCommands(command);
}

VkDescriptorImageInfo LveTexture::descriptorInfo() const {
  return {sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
}

LveTexture::~LveTexture() {
  vkDestroySampler(lveDevice.device(), sampler, nullptr);
  vkDestroyImageView(lveDevice.device(), imageView, nullptr);
  vkDestroyImage(lveDevice.device(), image, nullptr);
  vkFreeMemory(lveDevice.device(), memory, nullptr);
}
}  // namespace lve
