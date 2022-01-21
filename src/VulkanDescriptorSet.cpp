#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"

VulkanDescriptorSet::VulkanDescriptorSet(std::shared_ptr<VulkanDevice> device_, VkDescriptorSet descriptorSet_)
    : device(device_), descriptorSet(descriptorSet_) {
}

void VulkanDescriptorSet::WriteUniformBufferArray(int bindingIndex, std::vector<std::shared_ptr<VulkanBuffer>> buffers, int startOffset) {
    VkWriteDescriptorSet descriptorWrite;
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = bindingIndex;
    descriptorWrite.dstArrayElement = startOffset;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = buffers.size();

    vkUpdateDescriptorSets(device->Handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::WriteUniformBuffer(int bindingIndex, std::shared_ptr<VulkanBuffer> buffer, int startOffset) {
    WriteUniformBufferArray(bindingIndex, {buffer}, startOffset);
}
