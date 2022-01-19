#pragma once

#include "vk_common.h"

#include "VulkanCommandPool.h"

class VulkanCommandBuffer {
    VK_NON_COPIABLE(VulkanCommandBuffer)

public:
    VulkanCommandBuffer(VkCommandBuffer commandBuffer_);

private:
VK_HANDLE(VkCommandBuffer, commandBuffer)
};
