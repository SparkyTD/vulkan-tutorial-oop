#pragma once

#include "vk_common.h"

class VulkanTextureSampler {
    VK_NON_COPIABLE(VulkanTextureSampler)

public:
    VulkanTextureSampler(std::shared_ptr<VulkanInstance> instance_, std::shared_ptr<VulkanDevice> device_);

    ~VulkanTextureSampler();

private:
    std::shared_ptr<VulkanInstance> instance;
    std::shared_ptr<VulkanDevice> device;
VK_HANDLE(VkSampler, textureSampler);
};
