#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <cstdint>

#include "vk_common.h"
#include "VulkanWindow.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSetBuilder.h"
#include "VulkanDescriptorSet.h"
#include "VulkanTextureSampler.h"
#include "VulkanFramebuffer.h"
#include "Mesh.h"

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

const int MAX_FRAMES_IN_FLIGHT = 2;

class HelloTriangleApplication {
public:
    void run() {
        initVulkan();
        mainLoop();
    }

private:
    std::shared_ptr<VulkanWindow> window;
    std::shared_ptr<VulkanInstance> instance;
    std::shared_ptr<VulkanDevice> device;
    std::shared_ptr<VulkanSwapChain> swapChain;
    std::shared_ptr<VulkanRenderPass> renderPass;
    std::shared_ptr<VulkanGraphicsPipeline> graphicsPipeline;
    std::shared_ptr<VulkanCommandPool> commandPool;
    std::shared_ptr<VulkanDescriptorSetBuilder> descriptorSetBuilder;

    std::shared_ptr<VulkanImage> colorImage;
    std::shared_ptr<VulkanImage> depthImage;
    std::shared_ptr<VulkanImage> textureImage;
    std::shared_ptr<VulkanTextureSampler> textureSampler;

    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<VulkanBuffer> indexBuffer;
    std::shared_ptr<VulkanBuffer> vertexBuffer;

    std::vector<std::shared_ptr<VulkanFramebuffer>> swapChainFramebuffers;
    std::vector<std::shared_ptr<VulkanBuffer>> uniformBuffers;
    std::vector<std::shared_ptr<VulkanDescriptorSet>> descriptorSets;
    std::vector<std::shared_ptr<VulkanCommandBuffer>> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    void initVulkan() { // TODO
        window = std::make_shared<VulkanWindow>();
        instance = std::make_shared<VulkanInstance>(window);
        device = std::make_shared<VulkanDevice>(instance);
        swapChain = std::make_shared<VulkanSwapChain>(window, device, instance);
        renderPass = std::make_shared<VulkanRenderPass>(instance, device, swapChain);
        commandPool = std::make_shared<VulkanCommandPool>(QueueFamily::Graphics, device, instance);
        textureSampler = std::make_shared<VulkanTextureSampler>(instance, device);

        createUniformBuffers();

        textureImage = VulkanImage::LoadFrom(TEXTURE_PATH.c_str(), instance, device, commandPool);

        descriptorSetBuilder = std::make_shared<VulkanDescriptorSetBuilder>(device, swapChain->GetImageCount());
        descriptorSetBuilder->AddLayoutSlot(ShaderStage::Vertex, 0, ShaderResourceType::UniformBuffer, 1);
        descriptorSetBuilder->AddLayoutSlot(ShaderStage::Fragment, 1, ShaderResourceType::ImageSampler, 1);
        descriptorSets = descriptorSetBuilder->Build();
        for (int i = 0; i < swapChain->GetImageCount(); i++) {
            descriptorSets[i]->WriteUniformBuffer(0, uniformBuffers[i], sizeof(UniformBufferObject));
            descriptorSets[i]->WriteImage(1, textureSampler, textureImage->GetView(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT));
        }

        auto vertShaderModule = std::make_shared<VulkanShader>("shaders/vert.spv", device);
        auto fragShaderModule = std::make_shared<VulkanShader>("shaders/frag.spv", device);
        graphicsPipeline = std::make_shared<VulkanGraphicsPipeline>(vertShaderModule, fragShaderModule, renderPass, device, swapChain, descriptorSetBuilder->GetLayout());

        mesh = std::make_shared<Mesh>(MODEL_PATH.c_str());
        indexBuffer = mesh->CreateIndexBuffer(commandPool, instance, device);
        vertexBuffer = mesh->CreateVertexBuffer(commandPool, instance, device);

        createColorResources();
        createDepthResources();

        createFramebuffers();
        createCommandBuffers();
        createSyncObjects();
    }

    void mainLoop() {
        uint64_t sampleCount = 0;
        auto lastPrint = std::chrono::high_resolution_clock::now();
        while (!window->IsClosing()) {
            window->PollEvents();
            auto t1 = std::chrono::high_resolution_clock::now();
            drawFrame();
            sampleCount++;

            if (std::chrono::duration_cast<std::chrono::milliseconds>(t1 - lastPrint).count() > 1000) {
                printf("Avg. FPS = %lld\n", sampleCount);
                sampleCount = 0;
                lastPrint = t1;
            }
        }

        device->WaitIdle();
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        window->GetFramebufferSize(width, height);
        while (width == 0 || height == 0) {
            window->GetFramebufferSize(width, height);
            window->WaitForEvents();
        }

        device->WaitIdle();
        swapChain = std::make_shared<VulkanSwapChain>(window, device, instance);
        renderPass = std::make_shared<VulkanRenderPass>(instance, device, swapChain);

        createColorResources();
        createDepthResources();
        createFramebuffers();

        auto vertShaderModule = std::make_shared<VulkanShader>("shaders/vert.spv", device);
        auto fragShaderModule = std::make_shared<VulkanShader>("shaders/frag.spv", device);
        graphicsPipeline = std::make_shared<VulkanGraphicsPipeline>(vertShaderModule, fragShaderModule, renderPass, device, swapChain, descriptorSetBuilder->GetLayout());

        imagesInFlight.resize(swapChain->GetImageCount(), VK_NULL_HANDLE);
    }

    void createFramebuffers() {
        swapChainFramebuffers.clear();

        for (size_t i = 0; i < swapChain->GetImageCount(); i++) {
            std::vector<std::shared_ptr<VulkanImageView>> attachments = {
                colorImage->GetView(swapChain->GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT),
                depthImage->GetView(renderPass->FindDepthFormat(), VK_IMAGE_ASPECT_DEPTH_BIT),
                swapChain->GetImageView(i)
            };
            swapChainFramebuffers.push_back(std::make_shared<VulkanFramebuffer>(device, swapChain, renderPass, attachments));
        }
    }

    void createColorResources() {
        colorImage = std::make_shared<VulkanImage>(instance, device, swapChain->GetExtent().width, swapChain->GetExtent().height, VulkanInstance::MsaaSamples,
                                                   swapChain->GetFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
    }

    void createDepthResources() {
        depthImage = std::make_shared<VulkanImage>(instance, device, swapChain->GetExtent().width, swapChain->GetExtent().height, VulkanInstance::MsaaSamples,
                                                   renderPass->FindDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
    }

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        uniformBuffers.resize(swapChain->GetImageCount());

        for (size_t i = 0; i < swapChain->GetImageCount(); i++) {
            uniformBuffers[i] = std::make_shared<VulkanBuffer>(device, instance, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }

    void createCommandBuffers() {
        commandBuffers.clear();
        for (int i = 0; i < swapChainFramebuffers.size(); i++)
            commandBuffers.push_back(commandPool->AllocateBuffer());
    }

    void recordCommandBuffers() {
        // Each Swap Chain image has its own cmd buffer, so the same list of instructions must be recorded for all of them (Why?)
        for (size_t i = 0; i < commandBuffers.size(); i++) {
            commandBuffers[i]->Begin(); // Start recording into the command buffer corresponding to this swap-chain framebuffer
            {
                renderPass->Begin(commandBuffers[i], swapChainFramebuffers[i]);
                {
                    // Bind the Shader configuration (aka Pipeline)
                    graphicsPipeline->Bind(commandBuffers[i]);

                    // Bind the Mesh (vertexBuffer, indexBuffer)
                    VkBuffer vertexBuffers[] = {vertexBuffer->Handle()};
                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(commandBuffers[i]->Handle(), 0, 1, vertexBuffers, offsets);
                    vkCmdBindIndexBuffer(commandBuffers[i]->Handle(), indexBuffer->Handle(), 0, VK_INDEX_TYPE_UINT32);

                    // Bind the shader descriptor set (aka which resources belong to which shader layout slots)
                    descriptorSets[i]->Bind(commandBuffers[i], graphicsPipeline);

                    // Main Draw command
                    vkCmdDrawIndexed(commandBuffers[i]->Handle(), mesh->GetIndexCount(), 1, 0, 0, 0);
                }
                renderPass->End(commandBuffers[i]);
            }
            // End the recording of this command buffer. Don't submit to GPU yet, just keep it as a "list of instructions" that the render loop will execute each frame
            commandBuffers[i]->End();
        }
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(swapChain->GetImageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device->Handle(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device->Handle(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device->Handle(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count() * 0.1f;

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapChain->GetExtent().width / (float) swapChain->GetExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        uniformBuffers[currentImage]->CopyFrom(&ubo, sizeof(ubo));
    }

    void drawFrame() {
        recordCommandBuffers();

        vkWaitForFences(device->Handle(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device->Handle(), swapChain->Handle(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateUniformBuffer(imageIndex);

        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device->Handle(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        auto commandBuffer = commandBuffers[imageIndex]->Handle();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(device->Handle(), 1, &inFlightFences[currentFrame]);

        result = vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]);
        if (result != VK_SUCCESS) {
            if (result == VK_ERROR_DEVICE_LOST) {
                throw std::runtime_error("vkQueueSubmit(): VK_ERROR_DEVICE_LOST");
            } else {
                throw std::runtime_error("failed to submit draw command buffer!");
            }
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain->Handle()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(device->GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->IsWindowResized(true)) {
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}