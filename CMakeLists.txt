cmake_minimum_required(VERSION 3.20)
project(vulkan_tutorial)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glm REQUIRED FATAL_ERROR)

add_executable(vulkan_tutorial src/main.cpp.old src/vk_common.h src/main.cpp src/stb_image.h src/vk_forward.h src/VulkanWindow.cpp src/VulkanWindow.h src/VulkanInstance.cpp src/VulkanInstance.h src/vk_structures.h src/VulkanDevice.cpp src/VulkanDevice.h src/VulkanSwapChain.cpp src/VulkanSwapChain.h)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)
target_include_directories(${PROJECT_NAME} PUBLIC Vulkan::Headers)

target_link_libraries(${PROJECT_NAME} PUBLIC "D:/lib/glfw-3.3.6/bin/lib-vc2022/glfw3.lib")
target_include_directories(${PROJECT_NAME} PUBLIC "D:/lib/glfw-3.3.6/bin/include")