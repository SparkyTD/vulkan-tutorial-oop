#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer commandBuffer_, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanCommandPool> commandPool_)
    : commandBuffer(commandBuffer_), device(device_), commandPool(commandPool_) {
}

std::shared_ptr<VulkanCommandBuffer> VulkanCommandBuffer::Begin(bool singleTime) {
    isSingleTime = singleTime;
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (singleTime)
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    currentState = VulkanCommandBufferState::Recording;

    return this->shared_from_this();
}

void VulkanCommandBuffer::End() {
    vkEndCommandBuffer(commandBuffer);
    currentState = VulkanCommandBufferState::Executable;
}

void VulkanCommandBuffer::EndAndSubmit() {
    End();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->GetGraphicsQueue());
    currentState = VulkanCommandBufferState::Pending;

    if (!isFreed) {
        vkFreeCommandBuffers(device->Handle(), commandPool->Handle(), 1, &commandBuffer);
        isFreed = true;
    }
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    if (!isFreed) {
        vkFreeCommandBuffers(device->Handle(), commandPool->Handle(), 1, &commandBuffer);
        isFreed = true;
    }
}

void VulkanCommandBuffer::Reset() {
    vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}
