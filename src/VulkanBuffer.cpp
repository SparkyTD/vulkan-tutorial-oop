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
