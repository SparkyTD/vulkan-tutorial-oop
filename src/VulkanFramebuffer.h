#pragma once

#include "vk_common.h"

class VulkanFramebuffer {
    VK_NON_COPIABLE(VulkanFramebuffer)
public:
    // VulkanFramebuffer(std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanSwapChain> swapChain_, std::shared_ptr<VulkanRe> renderPass_);

private:
    std::shared_ptr<VulkanDevice> device;

private:

VK_HANDLE(VkFramebuffer, framebuffer);
};

