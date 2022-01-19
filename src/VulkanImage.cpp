#include "VulkanImage.h"

#include "VulkanImageView.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"

VulkanImage::VulkanImage(VkImage image_, std::shared_ptr<VulkanDevice> device_) : image(image_), device(device_) {

}

VulkanImage::VulkanImage(std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanDevice> device_,
                         uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
                         VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties) : device(device_) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device->Handle(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->Handle(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(instance, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device->Handle(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device->Handle(), image, imageMemory, 0);
}

std::shared_ptr<VulkanImageView> VulkanImage::GetView(VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
    std::hash<uint32_t> uint_hash;
    auto viewHash = uint_hash((uint32_t) format) + 0x9e3779b9 + uint_hash((uint32_t) aspectFlags) + 0x9e3779b1 + uint_hash(mipLevels);
    auto match = imageViewCache.find(viewHash);
    if (match != imageViewCache.end())
        return match->second;

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageViewHandle;
    if (vkCreateImageView(device->Handle(), &viewInfo, nullptr, &imageViewHandle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    auto imageView = std::make_shared<VulkanImageView>(imageViewHandle, this->shared_from_this());
    imageViewCache[viewHash] = imageView;

    return imageView;
}

uint32_t VulkanImage::FindMemoryType(std::shared_ptr<VulkanInstance> instance, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(instance->PhysicalDeviceHandle(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
