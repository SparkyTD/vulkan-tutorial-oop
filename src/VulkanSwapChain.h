#pragma once

#include "vk_common.h"

class VulkanSwapChain {
    VK_NON_COPIABLE(VulkanSwapChain)

public:
    VulkanSwapChain(std::shared_ptr<VulkanWindow> window_, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanInstance> instance_);

    ~VulkanSwapChain();

    std::vector<std::shared_ptr<VulkanImage>>& GetImages();

    std::vector<std::shared_ptr<VulkanImageView>>& GetImageViews();

    std::shared_ptr<VulkanImage> GetImage(int index);

    std::shared_ptr<VulkanImageView> GetImageView(int index);

    int GetImageCount();

    VkExtent2D GetExtent();

    VkFormat GetFormat();

    void CreateImageViews();

private:
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

private:
    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanInstance> instance;
    std::shared_ptr<VulkanWindow> window;

private:
    std::vector<std::shared_ptr<VulkanImageView>> imageViews;
    std::vector<std::shared_ptr<VulkanImage>> images;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

VK_HANDLE(VkSwapchainKHR, swapChain);
};
