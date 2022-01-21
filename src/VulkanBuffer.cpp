#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanImage.h"
#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"

VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanInstance> instance_,
                           VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : device(device_), instance(instance_) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device->Handle(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device->Handle(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanImage::FindMemoryType(instance, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device->Handle(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device->Handle(), buffer, bufferMemory, 0);
}

VkDeviceMemory VulkanBuffer::GetMemory() {
    return bufferMemory;
}

VulkanBuffer::~VulkanBuffer() {
    vkDestroyBuffer(device->Handle(), buffer, nullptr);
    vkFreeMemory(device->Handle(), bufferMemory, nullptr);
}

void VulkanBuffer::CopyTo(std::shared_ptr<VulkanCommandPool> commandPool, std::shared_ptr<VulkanBuffer> destination, VkDeviceSize size) {
    auto commandBuffer = commandPool->AllocateBuffer()->Begin();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer->Handle(), Handle(), destination->Handle(), 1, &copyRegion);

    commandBuffer->EndAndSubmit();
}

void VulkanBuffer::CopyFrom(void *inputData, int length) {
    void *data;
    vkMapMemory(device->Handle(), bufferMemory, 0, length, 0, &data);
    memcpy(data, inputData, length);
    vkUnmapMemory(device->Handle(), bufferMemory);
}

void VulkanBuffer::CopyTo(std::shared_ptr<VulkanCommandPool> commandPool, std::shared_ptr<VulkanImage> destination) {
    auto commandBuffer = commandPool->AllocateBuffer()->Begin();

    uint32_t width, height;
    destination->GetSize(width, height);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer->Handle(), buffer, destination->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    commandBuffer->EndAndSubmit();
}
