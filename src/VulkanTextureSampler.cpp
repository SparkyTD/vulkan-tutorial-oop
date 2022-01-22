#include "VulkanTextureSampler.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"

VulkanTextureSampler::VulkanTextureSampler(std::shared_ptr<VulkanInstance> instance_, std::shared_ptr<VulkanDevice> device_)
    : instance(instance_), device(device_) {

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(instance->PhysicalDeviceHandle(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE; // TODO MipMap level was fed in here when the Image created the sampler
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(device->Handle(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

VulkanTextureSampler::~VulkanTextureSampler() {
    vkDestroySampler(device->Handle(), textureSampler, nullptr);
}
