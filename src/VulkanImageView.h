#pragma once

#include "vk_common.h"

class VulkanImageView {
    VK_NON_COPIABLE(VulkanImageView)

public:
    VulkanImageView(VkImageView imageView_, std::shared_ptr<VulkanImage> image_);

    std::shared_ptr<VulkanImage> GetImage();

private:
    std::shared_ptr<VulkanImage> image;

VK_HANDLE(VkImageView, imageView);
};
