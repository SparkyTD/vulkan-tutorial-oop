#pragma once

#include "vk_common.h"

class VulkanBuffer {
    VK_NON_COPIABLE(VulkanBuffer)

public:
    VulkanBuffer(std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanInstance> instance_, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~VulkanBuffer();

    VkDeviceMemory GetMemory();

    void CopyTo(std::shared_ptr<VulkanCommandPool> commandPool, std::shared_ptr<VulkanBuffer> destination, VkDeviceSize size);

private:
    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanInstance> instance;

private:
    VkDeviceMemory bufferMemory;
VK_HANDLE(VkBuffer, buffer);
};
