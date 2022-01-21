#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanImageView.h"

VulkanDescriptorSet::VulkanDescriptorSet(std::shared_ptr<VulkanDevice> device_, VkDescriptorSet descriptorSet_)
    : device(device_), descriptorSet(descriptorSet_) {
}

void VulkanDescriptorSet::WriteUniformBuffer(int bindingIndex, std::shared_ptr<VulkanBuffer> buffer, int bufferSize) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer->Handle();
    bufferInfo.offset = 0;
    bufferInfo.range = bufferSize; // sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = bindingIndex;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device->Handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::WriteImage(int bindingIndex, VkSampler textureSampler, std::shared_ptr<VulkanImageView> imageView) {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView->Handle();
    imageInfo.sampler = textureSampler;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = bindingIndex;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device->Handle(), 1, &descriptorWrite, 0, nullptr);
}
