cmake_minimum_required(VERSION 3.20)
project(vulkan_tutorial)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glm REQUIRED FATAL_ERROR)

add_executable(vulkan_tutorial src/vk_common.h src/main.cpp src/stb_image.h src/vk_forward.h src/VulkanWindow.cpp src/VulkanWindow.h src/VulkanInstance.cpp src/VulkanInstance.h src/vk_structures.h src/VulkanDevice.cpp src/VulkanDevice.h src/VulkanSwapChain.cpp src/VulkanSwapChain.h src/VulkanFramebuffer.cpp src/VulkanFramebuffer.h src/VulkanRenderPass.cpp src/VulkanRenderPass.h src/VulkanShader.cpp src/VulkanShader.h src/VulkanGraphicsPipeline.cpp src/VulkanGraphicsPipeline.h src/VulkanCommandPool.cpp src/VulkanCommandPool.h src/VulkanCommandBuffer.cpp src/VulkanCommandBuffer.h src/VulkanImage.cpp src/VulkanImage.h src/VulkanImageView.cpp src/VulkanImageView.h src/VulkanBuffer.cpp src/VulkanBuffer.h src/VulkanDescriptorSet.cpp src/VulkanDescriptorSet.h src/VulkanDescriptorSetBuilder.cpp src/VulkanDescriptorSetBuilder.h src/VulkanTextureSampler.cpp src/VulkanTextureSampler.h src/VulkanMesh.cpp src/VulkanMesh.h src/vulkan-tutorial/multisampling_29.cpp src/vulkan-tutorial/multisampling_29.h src/lib_common.h src/VkValidationClient.cpp src/VkValidationClient.h)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)
target_include_directories(${PROJECT_NAME} PUBLIC Vulkan::Headers)

target_link_libraries(${PROJECT_NAME} PUBLIC "D:/lib/glfw-3.3.6/bin/lib-vc2022/glfw3.lib")
target_include_directories(${PROJECT_NAME} PUBLIC "D:/lib/glfw-3.3.6/bin/include")