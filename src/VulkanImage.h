#pragma once

#include "vk_common.h"

class VulkanImage : public std::enable_shared_from_this<VulkanImage> {
    VK_NON_COPIABLE(VulkanImage)

public:
    VulkanImage(VkImage image_, std::shared_ptr<VulkanDevice> device_);

    VulkanImage(std::shared_ptr<VulkanInstance> instance_, std::shared_ptr<VulkanDevice> device_,
                uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format,
                VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool useMipLevels);

    std::shared_ptr<VulkanImageView> GetView(VkFormat format, VkImageAspectFlags aspectFlags);

    void ChangeLayout(std::shared_ptr<VulkanCommandBuffer> commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

    void GetSize(uint32_t &width_, uint32_t &height_);

    void GenerateMipMaps(std::shared_ptr<VulkanCommandPool> commandPool);

public:
    static uint32_t FindMemoryType(std::shared_ptr<VulkanInstance> instance, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static std::shared_ptr<VulkanImage> LoadFrom(const char* path, std::shared_ptr<VulkanInstance> instance_, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanCommandPool> commandPool);

private:
    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanInstance> instance;

    std::unordered_map<uint32_t, std::shared_ptr<VulkanImageView>> imageViewCache;

    void CreateImageInternal(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format,
                                                 VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, int mipLevels);

private:
    VkFormat format;
    VkDeviceMemory imageMemory;
    uint32_t width, height;
    uint32_t mipLevels;

VK_HANDLE(VkImage, image);
};
