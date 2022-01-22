#pragma once

#include "vk_common.h"

#include "VulkanCommandPool.h"

enum class VulkanCommandBufferState {
    Initial,
    Recording,
    Executable,
    Pending,
    Invalid
};

class VulkanCommandBuffer : public std::enable_shared_from_this<VulkanCommandBuffer> {
    VK_NON_COPIABLE(VulkanCommandBuffer)

public:
    VulkanCommandBuffer(VkCommandBuffer commandBuffer_, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanCommandPool> commandPool_);

    ~VulkanCommandBuffer();

    std::shared_ptr<VulkanCommandBuffer> Begin(bool singleTime);

    void End();

    void EndAndSubmit();

    void Reset();

private:
    VulkanCommandBufferState currentState = VulkanCommandBufferState::Initial;

    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanCommandPool> commandPool;

    bool isSingleTime = false;
    bool isFreed = false;
VK_HANDLE(VkCommandBuffer, commandBuffer)
};
