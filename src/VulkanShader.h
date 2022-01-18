#pragma once

#include "vk_common.h"

class VulkanShader {
    VK_NON_COPIABLE(VulkanShader)

public:
    VulkanShader(const char* path, std::shared_ptr<VulkanDevice> device_);

private:
    static std::vector<char> ReadFile(const std::string &filename);

private:
    std::shared_ptr<VulkanDevice> device;

VK_HANDLE(VkShaderModule, shaderModule);
};
