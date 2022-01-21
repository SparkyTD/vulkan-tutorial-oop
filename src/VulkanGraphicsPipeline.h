#pragma once

#include "vk_common.h"

class VulkanGraphicsPipeline {
    VK_NON_COPIABLE(VulkanGraphicsPipeline)

public:
    VulkanGraphicsPipeline(std::shared_ptr<VulkanShader> vertexShader_, std::shared_ptr<VulkanShader> fragmentShader_,
                           std::shared_ptr<VulkanRenderPass> renderPass_, std::shared_ptr<VulkanDevice> device_,
                           std::shared_ptr<VulkanSwapChain> swapChain_, VkDescriptorSetLayout descriptorSetLayout);

    VkDescriptorSetLayout GetDescriptorSetLayout();
    VkPipelineLayout GetPipelineLayout();

private:
    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanShader> vertexShader;
    std::shared_ptr<VulkanShader> fragmentShader;
    std::shared_ptr<VulkanSwapChain> swapChain;
    std::shared_ptr<VulkanRenderPass> renderPass;

private:
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;

VK_HANDLE(VkPipeline, graphicsPipeline);
};

