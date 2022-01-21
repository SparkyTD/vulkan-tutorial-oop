#pragma once

#include "vk_common.h"

class VulkanImage : public std::enable_shared_from_this<VulkanImage> {
    VK_NON_COPIABLE(VulkanImage)

public:
    VulkanImage(VkImage image_, std::shared_ptr<VulkanDevice> device_);

    VulkanImage(std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanDevice> device_,
                uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
                VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

    std::shared_ptr<VulkanImageView> GetView(VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

public:
    static uint32_t FindMemoryType(std::shared_ptr<VulkanInstance> instance, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void ChangeLayout(std::shared_ptr<VulkanCommandBuffer> commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

private:
    std::shared_ptr<VulkanDevice> device;

    std::unordered_map<uint32_t, std::shared_ptr<VulkanImageView>> imageViewCache;

private:
    VkFormat format;
    VkDeviceMemory imageMemory;
VK_HANDLE(VkImage, image);
};
