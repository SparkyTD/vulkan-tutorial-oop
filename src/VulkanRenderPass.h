#pragma once

#include "vk_common.h"

class VulkanRenderPass {
    VK_NON_COPIABLE(VulkanRenderPass)

public:
    VulkanRenderPass(std::shared_ptr<VulkanInstance> instance_, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanSwapChain> swapChain_);

    VkFormat FindDepthFormat();

    void Begin(std::shared_ptr<VulkanCommandBuffer> commandBuffer, std::shared_ptr<VulkanFramebuffer> framebuffer);
    void End(std::shared_ptr<VulkanCommandBuffer> commandBuffer);

private:
    VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

private:
    std::shared_ptr<VulkanInstance> instance;
    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanSwapChain> swapChain;

private:
VK_HANDLE(VkRenderPass, renderPass);
};
