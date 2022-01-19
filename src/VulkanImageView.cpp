
#include "VulkanImageView.h"

VulkanImageView::VulkanImageView(VkImageView imageView_, std::shared_ptr<VulkanImage> image_) : imageView(imageView_), image(image_) {

}

std::shared_ptr<VulkanImage> VulkanImageView::GetImage() {
    return image;
}
