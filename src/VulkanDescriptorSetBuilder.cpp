#include "VulkanDescriptorSetBuilder.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"

VulkanDescriptorSetBuilder::VulkanDescriptorSetBuilder(std::shared_ptr<VulkanDevice> device_, int swapChainCount_)
    : swapChainCount(swapChainCount_), device(device_) {
}

VulkanDescriptorSetBuilder::~VulkanDescriptorSetBuilder() {
    layoutBindings.clear();
    poolSizes.clear();
}

void VulkanDescriptorSetBuilder::AddLayoutSlot(ShaderStage shaderStage, int slotIndex, ShaderResourceType type, int count) {
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = slotIndex;
    layoutBinding.descriptorCount = count;
    layoutBinding.descriptorType = (VkDescriptorType) type;
    layoutBinding.pImmutableSamplers = nullptr;
    layoutBinding.stageFlags = (VkShaderStageFlags) shaderStage;
    layoutBindings.push_back(layoutBinding);

    VkDescriptorPoolSize poolSize;
    poolSize.type = (VkDescriptorType) type;
    poolSize.descriptorCount = static_cast<uint32_t>(swapChainCount);
    poolSizes.push_back(poolSize);
}

std::vector<std::shared_ptr<VulkanDescriptorSet>> VulkanDescriptorSetBuilder::Build() {
    if (layoutBindings.empty()) {
        throw std::runtime_error("no layout slots have been added to this VulkanDescriptorSetBuilder");
    }

    // Descriptor Set Layout
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(device->Handle(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    // Descriptor Pool
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(swapChainCount);

    VkDescriptorPool descriptorPool;
    if (vkCreateDescriptorPool(device->Handle(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    // Allocate Pools
    std::vector<VkDescriptorSetLayout> layouts(swapChainCount, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = swapChainCount;
    allocInfo.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> descriptorSetHandles;
    descriptorSetHandles.resize(swapChainCount);
    if (vkAllocateDescriptorSets(device->Handle(), &allocInfo, descriptorSetHandles.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    std::vector<std::shared_ptr<VulkanDescriptorSet>> descriptorSets;
    for(int i = 0; i < swapChainCount; i++) {
        descriptorSets.push_back(std::make_shared<VulkanDescriptorSet>(device, descriptorSetHandles[i]));
    }

    return descriptorSets;
}

VkDescriptorSetLayout VulkanDescriptorSetBuilder::GetLayout() {
    return descriptorSetLayout;
}
