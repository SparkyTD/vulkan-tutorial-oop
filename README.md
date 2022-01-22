# vulkan-tutorial-oop

An attempt to rewrite the end-result of the [vulkan-tutorial.com](https://vulkan-tutorial.com/) tutorial code in an easier to understand way, by abstracting all the vulkan objects into their own classes. This way the original 1.7k line main.cpp file could be reduced to ~250 lines.

### Implemented Abstraction Classes:
- VulkanBuffer
- VulkanCommandBuffer
- VulkanCommandPool
- VulkanDescriptorSet
- VulkanDescriptorSetBuilder
- VulkanDevice
- VulkanFramebuffer
- VulkanGraphicsPipeline
- VulkanImage
- VulkanImageView
- VulkanInstance
- VulkanRenderPass
- VulkanShader
- VulkanSwapChain
- VulkanTextureSampler
- VulkanWindow
