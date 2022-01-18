#pragma once

#include "vk_common.h"

class VulkanDevice {
    VK_NON_COPIABLE(VulkanDevice)

public:
    VulkanDevice(std::shared_ptr<VulkanInstance> instance_);

    ~VulkanDevice();

    void WaitIdle();

    VkQueue GetGraphicsQueue();
    VkQueue GetPresentQueue();

private:
    std::shared_ptr<VulkanInstance> instance;

private:
    VkQueue graphicsQueue;
    VkQueue presentQueue;
VK_HANDLE(VkDevice, device);
};
