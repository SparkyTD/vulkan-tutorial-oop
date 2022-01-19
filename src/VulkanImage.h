#pragma once

#include "vk_common.h"

class VulkanImage {
    VK_NON_COPIABLE(VulkanImage)

public:
    VulkanImage(VkImage image_);

private:

VK_HANDLE(VkImage, image);
};
