#pragma once

#include "vk_common.h"

enum class ShaderResourceType {
    UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    ImageSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
};

class VulkanDescriptorSetBuilder {
    VK_NON_COPIABLE(VulkanDescriptorSetBuilder)

public:
    VulkanDescriptorSetBuilder(std::shared_ptr<VulkanDevice> device_, int swapChainCount_);
    ~VulkanDescriptorSetBuilder();

    void AddLayoutSlot(ShaderStage shaderStage, int slotIndex, ShaderResourceType type, int count);

    std::vector<std::shared_ptr<VulkanDescriptorSet>> Build();

    VkDescriptorSetLayout GetLayout();



private:
    std::shared_ptr<VulkanDevice> device;
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    std::vector<VkDescriptorPoolSize> poolSizes;
    int swapChainCount = -1;

    VkDescriptorSetLayout descriptorSetLayout;
};
