#include "VulkanImage.h"

#include "VulkanImageView.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanTextureSampler.h"

#include "lib_common.h"

VulkanImage::VulkanImage(VkImage image_, std::shared_ptr<VulkanDevice> device_) : image(image_), device(device_), mipLevels(1) {

}

VulkanImage::VulkanImage(std::shared_ptr<VulkanInstance> instance_, std::shared_ptr<VulkanDevice> device_,
                         uint32_t width_, uint32_t height_, VkSampleCountFlagBits numSamples, VkFormat format_,
                         VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties, bool useMipLevels)
    : device(device_), format(format_), width(width_), height(height_), instance(instance_) {

    if (useMipLevels)
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
    else
        mipLevels = 1;

    CreateImageInternal(width, height, numSamples, format, tiling, usage, properties, mipLevels);
}

std::shared_ptr<VulkanImage> VulkanImage::LoadFrom(const char *path, std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanDevice> device,
                                                   std::shared_ptr<VulkanCommandPool> commandPool) {

    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    auto stagingBuffer = std::make_shared<VulkanBuffer>(device, instance, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer->CopyFrom(pixels, static_cast<size_t>(imageSize));
    stbi_image_free(pixels);

    auto textureImage = std::make_shared<VulkanImage>(instance, device, texWidth, texHeight, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);

    auto commandBuffer = commandPool->AllocateBuffer()->Begin(true);
    textureImage->ChangeLayout(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    commandBuffer->EndAndSubmit();

    stagingBuffer->CopyTo(commandPool, textureImage);

    textureImage->GenerateMipMaps(commandPool);
    return textureImage;
}

std::shared_ptr<VulkanImageView> VulkanImage::GetView(VkFormat format, VkImageAspectFlags aspectFlags) {
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

void VulkanImage::ChangeLayout(std::shared_ptr<VulkanCommandBuffer> commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer->Handle(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanImage::GetSize(uint32_t &width_, uint32_t &height_) {
    width_ = width;
    height_ = height;
}

void VulkanImage::GenerateMipMaps(std::shared_ptr<VulkanCommandPool> commandPool) {
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(instance->PhysicalDeviceHandle(), format, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    auto commandBuffer = commandPool->AllocateBuffer()->Begin(true);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = width;
    int32_t mipHeight = height;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer->Handle(),
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer->Handle(),
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer->Handle(),
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer->Handle(),
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    commandBuffer->EndAndSubmit();
}

void VulkanImage::CreateImageInternal(uint32_t width_, uint32_t height_, VkSampleCountFlagBits numSamples, VkFormat format_,
                                      VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, int mipLevels_) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width_;
    imageInfo.extent.height = height_;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels_;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format_;
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
