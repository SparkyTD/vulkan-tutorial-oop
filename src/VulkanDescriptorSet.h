#pragma once

#include "vk_common.h"

class VulkanDescriptorSet {
    VK_NON_COPIABLE(VulkanDescriptorSet)

public:
    VulkanDescriptorSet(std::shared_ptr<VulkanDevice> device_, VkDescriptorSet descriptorSet_);

    void WriteUniformBufferArray(int bindingIndex, std::vector<std::shared_ptr<VulkanBuffer>> buffers, int startOffset = 0);
    void WriteUniformBuffer(int bindingIndex, std::shared_ptr<VulkanBuffer> buffer, int startOffset = 0);

private:
    std::shared_ptr<VulkanDevice> device;

VK_HANDLE(VkDescriptorSet, descriptorSet);
};


