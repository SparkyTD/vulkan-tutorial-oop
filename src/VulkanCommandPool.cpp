#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"

VulkanCommandPool::VulkanCommandPool(QueueFamily queueFamily, std::shared_ptr<VulkanDevice> device_, std::shared_ptr<VulkanInstance> instance_)
    : device(device_), instance(instance_) {

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = instance->GetQueueFamilyIndex(queueFamily);

    if (vkCreateCommandPool(device->Handle(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}
