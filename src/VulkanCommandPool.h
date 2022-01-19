#pragma once

#include "vk_common.h"

class VulkanCommandPool {
    VK_NON_COPIABLE(VulkanCommandPool)

public:
    VulkanCommandPool(QueueFamily queueType, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanInstance> instance_);

    std::shared_ptr<VulkanCommandBuffer> AllocateBuffer();

private:
    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanInstance> instance;

VK_HANDLE(VkCommandPool, commandPool);
};
