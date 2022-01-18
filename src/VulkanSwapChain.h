#pragma once

#include "vk_common.h"

class VulkanSwapChain {
    VK_NON_COPIABLE(VulkanSwapChain)

public:
    VulkanSwapChain(std::shared_ptr<VulkanWindow> window_, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanInstance> instance_);

    ~VulkanSwapChain();

    void Reset();

    std::vector<VkImage>& GetImages();

    std::vector<VkImageView>& GetImageViews();

    VkImage GetImage(int index);

    VkImageView &GetImageView(int index);

    int GetImageCount();

    VkExtent2D GetExtent();

    VkFormat GetFormat();

private:
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

private:
    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanInstance> instance;
    std::shared_ptr<VulkanWindow> window;

private:
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

VK_HANDLE(VkSwapchainKHR, swapChain);
};
