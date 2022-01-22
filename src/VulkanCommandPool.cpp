#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanCommandBuffer.h"

VulkanCommandPool::VulkanCommandPool(QueueFamily queueFamily, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanInstance> instance_)
    : device(device_), instance(instance_) {

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = instance->GetQueueFamilyIndex(queueFamily);

    if (vkCreateCommandPool(device->Handle(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

std::shared_ptr<VulkanCommandBuffer> VulkanCommandPool::AllocateBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBufferHandle;
    if (vkAllocateCommandBuffers(device->Handle(), &allocInfo, &commandBufferHandle) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    return std::make_shared<VulkanCommandBuffer>(commandBufferHandle, device, this->shared_from_this());
}
