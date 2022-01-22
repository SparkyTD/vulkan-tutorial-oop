#include "VulkanSwapChain.h"

#include <algorithm>

#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanWindow.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanCommandBuffer.h"

VulkanSwapChain::VulkanSwapChain(std::shared_ptr<VulkanWindow> window_, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanInstance> instance_)
    : window(window_), device(device_), instance(instance_) {
    SwapChainSupportDetails swapChainSupport = instance->QuerySwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = std::max(swapChainSupport.capabilities.minImageCount, std::min(swapImageCount, swapChainSupport.capabilities.maxImageCount));

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

    // Create Synchronization objects
    imageAvailableSemaphores.resize(swapImageCount);
    renderFinishedSemaphores.resize(swapImageCount);
    inFlightFences.resize(swapImageCount);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < swapImageCount; i++) {
        if (vkCreateSemaphore(device->Handle(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device->Handle(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device->Handle(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
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
    // VSync ON = VK_PRESENT_MODE_FIFO_KHR
    // VSync OFF = VK_PRESENT_MODE_MAILBOX_KHR

    return VK_PRESENT_MODE_MAILBOX_KHR;
    //return VK_PRESENT_MODE_FIFO_KHR;
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

int VulkanSwapChain::GetImageCount() {
    return swapImageCount;
}

VkExtent2D VulkanSwapChain::GetExtent() {
    return swapChainExtent;
}

std::shared_ptr<VulkanImageView> VulkanSwapChain::GetImageView(int index) {
    return imageViews[index];
}

VkFormat VulkanSwapChain::GetFormat() {
    return swapChainImageFormat;
}

void VulkanSwapChain::AcquireNextImage() {
    vkWaitForFences(device->Handle(), 1, &inFlightFences[currentImageIndex], VK_TRUE, UINT64_MAX);
    lastAcquireResult = vkAcquireNextImageKHR(device->Handle(), swapChain, UINT64_MAX, imageAvailableSemaphores[currentImageIndex], VK_NULL_HANDLE, &currentImageIndex);

    if (lastAcquireResult != VK_SUCCESS && lastAcquireResult != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
}

void VulkanSwapChain::SubmitCommands(std::shared_ptr<VulkanCommandBuffer> commandBuffer) {
    auto commandBufferHandle = commandBuffer->Handle();
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentImageIndex]; // Wait for vkAcquireNextImageKHR to signal the semaphore
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBufferHandle;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentImageIndex]; // Signal this semaphore when rendering is finished

    vkResetFences(device->Handle(), 1, &inFlightFences[currentImageIndex]); // Reset this fence before giving it to vkQueueSubmit
    lastSubmitResult = vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentImageIndex]); // Signal this CPU fence after all the commands have executed
    if (lastSubmitResult != VK_SUCCESS) {
        if (lastSubmitResult == VK_ERROR_DEVICE_LOST) {
            throw std::runtime_error("vkQueueSubmit(): VK_ERROR_DEVICE_LOST");
        } else {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
    }
}

void VulkanSwapChain::Present() {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentImageIndex]; // Present begins after rendering commands have been executed
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &currentImageIndex;
    lastPresentResult = vkQueuePresentKHR(device->GetPresentQueue(), &presentInfo);

    if (lastPresentResult != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

bool VulkanSwapChain::IsInvalid() const {
    if (lastAcquireResult == VK_ERROR_OUT_OF_DATE_KHR)
        return true;

    if (lastPresentResult == VK_ERROR_OUT_OF_DATE_KHR || lastPresentResult == VK_SUBOPTIMAL_KHR)
        return true;

    return false;
}

uint32_t VulkanSwapChain::GetCurrentImage() const {
    return currentImageIndex;
}

VulkanSwapChain::~VulkanSwapChain() {

}
