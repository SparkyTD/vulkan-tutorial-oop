#pragma once

#include "vk_common.h"

#include "VulkanCommandPool.h"

class VulkanCommandBuffer : public std::enable_shared_from_this<VulkanCommandBuffer> {
    VK_NON_COPIABLE(VulkanCommandBuffer)

public:
    VulkanCommandBuffer(VkCommandBuffer commandBuffer_, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanCommandPool> commandPool_);
    ~VulkanCommandBuffer();

    std::shared_ptr<VulkanCommandBuffer> Begin();

    void End();
    void EndAndSubmit();

private:
    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanCommandPool> commandPool;

VK_HANDLE(VkCommandBuffer, commandBuffer)
};
