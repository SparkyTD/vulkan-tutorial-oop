#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include "stb_image.h"
#include "tiny_obj_loader.h"

#include "vulkan-tutorial/multisampling_29.h"

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

// #include "vulkan-tutorial/multisampling_29.h"

class HelloTriangleApplication {
private:
    const std::string MODEL_PATH = "models/viking_room.obj";
    const std::string TEXTURE_PATH = "textures/viking_room.png";

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
    std::shared_ptr<VulkanGraphicsPipeline> texturedGraphicsPipeline;
    std::shared_ptr<VulkanCommandPool> commandPool;
    std::shared_ptr<VulkanDescriptorSetBuilder> descriptorSetBuilder;

    std::shared_ptr<VulkanImage> colorImage;
    std::shared_ptr<VulkanImage> depthImage;

    std::shared_ptr<VulkanImage> textureImage;
    std::shared_ptr<VulkanTextureSampler> textureSampler;

    std::shared_ptr<Mesh> mesh;

    std::vector<std::shared_ptr<VulkanFramebuffer>> swapChainFramebuffers;
    std::vector<std::shared_ptr<VulkanBuffer>> uniformBuffers;
    std::vector<std::shared_ptr<VulkanDescriptorSet>> descriptorSets;
    std::vector<std::shared_ptr<VulkanCommandBuffer>> commandBuffers;

    void initVulkan() { // TODO
        window = std::make_shared<VulkanWindow>();
        instance = std::make_shared<VulkanInstance>(window);
        device = std::make_shared<VulkanDevice>(instance);
        swapChain = std::make_shared<VulkanSwapChain>(window, device, instance);
        renderPass = std::make_shared<VulkanRenderPass>(instance, device, swapChain);
        commandPool = std::make_shared<VulkanCommandPool>(QueueFamily::Graphics, device, instance);
        textureSampler = std::make_shared<VulkanTextureSampler>(instance, device);
        textureImage = VulkanImage::LoadFrom(TEXTURE_PATH.c_str(), instance, device, commandPool);

        createUniformBuffers();

        descriptorSetBuilder = std::make_shared<VulkanDescriptorSetBuilder>(device, swapChain->GetImageCount());
        descriptorSetBuilder->AddLayoutSlot(ShaderStage::Vertex, 0, ShaderResourceType::UniformBuffer, 1);
        descriptorSetBuilder->AddLayoutSlot(ShaderStage::Fragment, 1, ShaderResourceType::ImageSampler, 1);
        descriptorSets = descriptorSetBuilder->Build();
        for (int i = 0; i < swapChain->GetImageCount(); i++) {
            descriptorSets[i]->WriteUniformBuffer(0, uniformBuffers[i], sizeof(UniformBufferObject));
            descriptorSets[i]->WriteImage(1, textureSampler, textureImage->GetView(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT));
        }

        texturedGraphicsPipeline = std::make_shared<VulkanGraphicsPipeline>(
            std::make_shared<VulkanShader>("shaders/vert.spv", device),
            std::make_shared<VulkanShader>("shaders/frag.spv", device),
            renderPass, device, swapChain, descriptorSetBuilder->GetLayout());

        mesh = std::make_shared<Mesh>(MODEL_PATH.c_str());
        mesh->CreateBuffers(commandPool, instance, device);

        createColorResources();
        createDepthResources();

        createFramebuffers();
        createCommandBuffers();
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

        texturedGraphicsPipeline = std::make_shared<VulkanGraphicsPipeline>(
            std::make_shared<VulkanShader>("shaders/vert.spv", device),
            std::make_shared<VulkanShader>("shaders/frag.spv", device),
            renderPass, device, swapChain, descriptorSetBuilder->GetLayout());

        createColorResources();
        createDepthResources();
        createFramebuffers();
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

    void recordCommandBuffers(uint32_t imageIndex) {
        // commandBuffers[imageIndex]->Reset();
        commandBuffers[imageIndex]->Begin(false);
        {
            renderPass->Begin(commandBuffers[imageIndex], swapChainFramebuffers[imageIndex]);
            {
                // Bind the Shader configuration (aka Pipeline)
                texturedGraphicsPipeline->Bind(commandBuffers[imageIndex]);

                // Bind the shader descriptor set (aka which resources belong to which shader layout slots)
                descriptorSets[0]->Bind(commandBuffers[imageIndex], texturedGraphicsPipeline);

                // Bind the Mesh
                mesh->Bind(commandBuffers[imageIndex]);

                // Main Draw command
                mesh->Draw(commandBuffers[imageIndex]);
            }
            renderPass->End(commandBuffers[imageIndex]);
        }
        commandBuffers[imageIndex]->End();
    }

    void drawFrame() {
        swapChain->WaitForLastSubmit();
        // vkResetCommandBuffer(commandBuffers[currentImage]->Handle(), VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        swapChain->AcquireNextImage();
        int currentImage = swapChain->GetCurrentImage();

        updateUniformBuffer(currentImage);
        recordCommandBuffers(currentImage);

        if (swapChain->IsInvalid() || window->IsWindowResized(true)) {
            recreateSwapChain();
            return;
        }

        // Submit
        swapChain->SubmitCommands(commandBuffers[currentImage]);

        // Present
        swapChain->Present();

        if (swapChain->IsInvalid() || window->IsWindowResized(true)) {
            recreateSwapChain();
        }
    }
};

int main(int argc, char **argv) {

    bool runApp29 = argc >= 2 && strcmp(argv[1], "-app29") == 0;

    VulkanTutorial::multisampling_29 app29;
    HelloTriangleApplication app;

    try {
        if (runApp29)
            app29.run();
        else
            app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}