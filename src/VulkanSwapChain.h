#pragma once

#include "vk_common.h"

class VulkanSwapChain {
    VK_NON_COPIABLE(VulkanSwapChain)

public:
    VulkanSwapChain(std::shared_ptr<VulkanWindow> window_, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanInstance> instance_);

    ~VulkanSwapChain();

    void AcquireNextImage();

    void SubmitCommands(std::shared_ptr<VulkanCommandBuffer> commandBuffer);

    void Present();

    std::shared_ptr<VulkanImageView> GetImageView(int index);

    int GetImageCount();

    VkExtent2D GetExtent();

    VkFormat GetFormat();

    bool IsInvalid() const;

    uint32_t GetCurrentImage() const;

private:
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

private:
    const uint32_t swapImageCount = 2;

    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanInstance> instance;
    std::shared_ptr<VulkanWindow> window;

    std::vector<std::shared_ptr<VulkanImageView>> imageViews;
    std::vector<std::shared_ptr<VulkanImage>> images;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    uint32_t currentImageIndex = 0;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    VkResult lastAcquireResult = VK_SUCCESS;
    VkResult lastSubmitResult = VK_SUCCESS;
    VkResult lastPresentResult = VK_SUCCESS;


VK_HANDLE(VkSwapchainKHR, swapChain);
};
