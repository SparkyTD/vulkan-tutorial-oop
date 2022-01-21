#pragma once

#include "vk_common.h"

class VulkanDescriptorSet {
    VK_NON_COPIABLE(VulkanDescriptorSet)

public:
    VulkanDescriptorSet(std::shared_ptr<VulkanDevice> device_, VkDescriptorSet descriptorSet_);

    void WriteUniformBuffer(int bindingIndex, std::shared_ptr<VulkanBuffer> buffer, int bufferSize);

    void WriteImage(int bindingIndex, std::shared_ptr<VulkanTextureSampler> textureSampler, std::shared_ptr<VulkanImageView> imageView);

private:
    std::shared_ptr<VulkanDevice> device;

VK_HANDLE(VkDescriptorSet, descriptorSet);
};


