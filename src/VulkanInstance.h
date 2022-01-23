#pragma once

#include "vk_common.h"
#include "VkValidationClient.h"

class VulkanInstance {
    VK_NON_COPIABLE(VulkanInstance)

public:
    VulkanInstance(std::shared_ptr<VulkanWindow> window_);

    ~VulkanInstance();

    QueueFamilyIndices FindQueueFamilies();

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    uint32_t GetQueueFamilyIndex(QueueFamily queueFamily);

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    SwapChainSupportDetails QuerySwapChainSupport();

    VkInstance InstanceHandle();

    VkPhysicalDevice PhysicalDeviceHandle();

    VkSurfaceKHR SurfaceHandle();

private:
    std::shared_ptr<VulkanWindow> window;

private:
    bool CheckValidationLayerSupport();

    std::vector<const char *> GetRequiredExtensions();

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    void SetupDebugMessenger();

    void PickPhysicalDevice();

    VkSampleCountFlagBits GetMaxUsableSampleCount();

    bool IsDeviceSuitable(VkPhysicalDevice device);

    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

public:
    static const std::vector<const char *> ValidationLayers;
    static const std::vector<const char *> DeviceExtensions;
    static const bool EnableValidationLayers;
    static VkSampleCountFlagBits MsaaSamples;

private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                                 VkDebugUtilsMessengerEXT *pDebugMessenger);

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

private:
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    static VkValidationClient debugClient;

VK_HANDLE(VkInstance, instance);
};

