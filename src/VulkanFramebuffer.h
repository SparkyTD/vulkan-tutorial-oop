#pragma once

#include "vk_common.h"

class VulkanFramebuffer {
    VK_NON_COPIABLE(VulkanFramebuffer)

public:
    VulkanFramebuffer(std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanSwapChain> swapChain_,
                      std::shared_ptr<VulkanRenderPass> renderPass_, std::vector<std::shared_ptr<VulkanImageView>> attachments);

private:
    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanSwapChain> swapChain;
    std::shared_ptr<VulkanRenderPass> renderPass;

private:
VK_HANDLE(VkFramebuffer, framebuffer);
};

