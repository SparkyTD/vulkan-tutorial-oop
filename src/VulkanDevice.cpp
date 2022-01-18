#include "VulkanDevice.h"
#include "VulkanInstance.h"

VulkanDevice::VulkanDevice(std::shared_ptr<VulkanInstance> instance_) : instance(instance_) {
    QueueFamilyIndices indices = instance->FindQueueFamilies();

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(VulkanInstance::DeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = VulkanInstance::DeviceExtensions.data();

    if (VulkanInstance::EnableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanInstance::ValidationLayers.size());
        createInfo.ppEnabledLayerNames = VulkanInstance::ValidationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(instance->PhysicalDeviceHandle(), &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void VulkanDevice::WaitIdle() {
    vkDeviceWaitIdle(device);
}

VkQueue VulkanDevice::GetGraphicsQueue() {
    return graphicsQueue;
}

VkQueue VulkanDevice::GetPresentQueue() {
    return presentQueue;
}

VulkanDevice::~VulkanDevice() {
    vkDestroyDevice(device, nullptr);
}