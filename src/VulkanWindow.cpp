#include "VulkanWindow.h"

VulkanWindow *VulkanWindow::instance = nullptr;

VulkanWindow::VulkanWindow() {
    instance = this;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
}

void VulkanWindow::FramebufferResizeCallback(GLFWwindow *, int, int) {
    instance->framebufferResized = true;
}

bool VulkanWindow::IsWindowResized(bool reset) {
    bool flag = framebufferResized ? true : false;
    if (reset)
        framebufferResized = false;
    return flag;
}

bool VulkanWindow::IsClosing() const {
    return glfwWindowShouldClose(window);
}

void VulkanWindow::PollEvents() {
    glfwPollEvents();
}

void VulkanWindow::GetFramebufferSize(int &width, int &height) {
    glfwGetFramebufferSize(window, &width, &height);
}

void VulkanWindow::WaitForEvents() {
    glfwWaitEvents();
}

VulkanWindow::~VulkanWindow() {
    glfwDestroyWindow(window);
    glfwTerminate();

    window = nullptr;
    instance = nullptr;
}
