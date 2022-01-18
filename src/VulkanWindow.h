#pragma once

#include "vk_common.h"

class VulkanWindow {
    VK_NON_COPIABLE(VulkanWindow)

public:
    VulkanWindow();
    ~VulkanWindow();

    bool IsWindowResized(bool reset);
    bool IsClosing() const;
    void PollEvents();
    void GetFramebufferSize(int& width, int& height);
    void WaitForEvents();

private:
    static VulkanWindow* instance;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    bool framebufferResized = false;

private:
    static void FramebufferResizeCallback(GLFWwindow *window, int width, int height);

VK_HANDLE(GLFWwindow*, window);
};
