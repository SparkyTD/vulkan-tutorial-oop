#include "VulkanFramebuffer.h"
#include "VulkanImageView.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"

VulkanFramebuffer::VulkanFramebuffer(std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanSwapChain> swapChain_,
                                     std::shared_ptr<VulkanRenderPass> renderPass_, std::vector<std::shared_ptr<VulkanImageView>> attachments)
    : device(device_), swapChain(swapChain_), renderPass(renderPass_) {

    std::vector<VkImageView> attachmentHandles;
    for (const auto &attachment: attachments)
        attachmentHandles.push_back(attachment->Handle());

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass->Handle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentHandles.size());
    framebufferInfo.pAttachments = attachmentHandles.data();
    framebufferInfo.width = swapChain->GetExtent().width;
    framebufferInfo.height = swapChain->GetExtent().height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device->Handle(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}
