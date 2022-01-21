#include "VulkanSwapChain.h"

#include <algorithm>

#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanWindow.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"

VulkanSwapChain::VulkanSwapChain(std::shared_ptr<VulkanWindow> window_, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanInstance> instance_)
    : window(window_), device(device_), instance(instance_) {
    SwapChainSupportDetails swapChainSupport = instance->QuerySwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = instance->SurfaceHandle();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = instance->FindQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device->Handle(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    auto imageHandles = VkEnumerateVector(device->Handle(), swapChain, vkGetSwapchainImagesKHR);
    for (const auto &handle: imageHandles) {
        auto image = std::make_shared<VulkanImage>(handle, device);
        images.push_back(image);
        imageViews.push_back(image->GetView(swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT));
    }
}

VkSurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat: availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode: availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        window->GetFramebufferSize(width, height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

std::vector<std::shared_ptr<VulkanImage>> &VulkanSwapChain::GetImages() {
    return images;
}

std::vector<std::shared_ptr<VulkanImageView>> &VulkanSwapChain::GetImageViews() {
    return imageViews;
}

int VulkanSwapChain::GetImageCount() {
    return images.size();
}

VkExtent2D VulkanSwapChain::GetExtent() {
    return swapChainExtent;
}

std::shared_ptr<VulkanImage> VulkanSwapChain::GetImage(int index) {
    return images[index];
}

std::shared_ptr<VulkanImageView> VulkanSwapChain::GetImageView(int index) {
    return imageViews[index];
}

VkFormat VulkanSwapChain::GetFormat() {
    return swapChainImageFormat;
}

void VulkanSwapChain::CreateImageViews() {
    // imageViews.resize(images.size());
    // for (int i = 0; i < images.size(); i++)
    //     imageViews[i] = createImageView(images[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

VulkanSwapChain::~VulkanSwapChain() {

}
