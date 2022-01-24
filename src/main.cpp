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
#include "VulkanMesh.h"

#include <immintrin.h>
#include <xmmintrin.h>

glm::mat4 quatToMat(glm::vec4 q);
glm::vec4 angleAxisToQuat(float angle, glm::vec3 axis);
glm::vec4 lookAt(glm::vec3 eye, glm::vec3 center);

class HelloTriangleApplication {
private:
    const std::string CUBE_MODEL_PATH = "models/cube.obj";
    const std::string ROOM_MODEL_PATH = "models/viking_room.obj";
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

    std::shared_ptr<VulkanMesh> roomMesh;
    std::shared_ptr<VulkanMesh> cubeMesh;

    std::vector<std::shared_ptr<VulkanFramebuffer>> swapChainFramebuffers;
    std::vector<std::shared_ptr<VulkanBuffer>> uniformBuffers;
    std::vector<std::shared_ptr<VulkanDescriptorSet>> descriptorSets;
    std::vector<std::shared_ptr<VulkanCommandBuffer>> commandBuffers;

    void initVulkan() { // TODO
        window = std::make_shared<VulkanWindow>();
        instance = std::make_shared<VulkanInstance>(window);

        device = std::make_shared<VulkanDevice>(instance);
        commandPool = std::make_shared<VulkanCommandPool>(QueueFamily::Graphics, device, instance);
        textureSampler = std::make_shared<VulkanTextureSampler>(instance, device);

        loadResources();
        createUniformBuffers();

        descriptorSetBuilder = std::make_shared<VulkanDescriptorSetBuilder>(device, swapChain->GetImageCount());
        descriptorSetBuilder->AddLayoutSlot(ShaderStage::Vertex, 0, ShaderResourceType::UniformBuffer, 1);
        descriptorSetBuilder->AddLayoutSlot(ShaderStage::Fragment, 1, ShaderResourceType::ImageSampler, 1);
        descriptorSets = descriptorSetBuilder->Build();
        for (int i = 0; i < swapChain->GetImageCount(); i++) {
            descriptorSets[i]->WriteUniformBuffer(0, uniformBuffers[i], sizeof(UniformBufferObject));
            descriptorSets[i]->WriteImage(1, textureSampler, textureImage->GetView(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT));
        }

        createGraphicsPipeline();
        createFramebufferResources();
        createFramebuffers();

        createCommandBuffers();
    }

    void recreateSwapChain() {
        swapChain = std::make_shared<VulkanSwapChain>(window, device, instance);
        renderPass = std::make_shared<VulkanRenderPass>(instance, device, swapChain);

        createGraphicsPipeline();
        createFramebufferResources();
        createFramebuffers();
        createCommandBuffers();
    }

    void createGraphicsPipeline() {
        texturedGraphicsPipeline = std::make_shared<VulkanGraphicsPipeline>(
            std::make_shared<VulkanShader>("shaders/vert.spv", device),
            std::make_shared<VulkanShader>("shaders/frag.spv", device),
            renderPass, device, swapChain, descriptorSetBuilder->GetLayout());
    }

    void loadResources() {
        textureImage = VulkanImage::LoadFrom(TEXTURE_PATH.c_str(), instance, device, commandPool);

        roomMesh = std::make_shared<VulkanMesh>(ROOM_MODEL_PATH.c_str());
        roomMesh->CreateBuffers(commandPool, instance, device);

        cubeMesh = std::make_shared<VulkanMesh>(CUBE_MODEL_PATH.c_str());
        cubeMesh->CreateBuffers(commandPool, instance, device);

        swapChain = std::make_shared<VulkanSwapChain>(window, device, instance);
        renderPass = std::make_shared<VulkanRenderPass>(instance, device, swapChain);
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

    void createFramebufferResources() {
        colorImage = std::make_shared<VulkanImage>(instance, device, swapChain->GetExtent().width, swapChain->GetExtent().height, VulkanInstance::MsaaSamples,
                                                   swapChain->GetFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
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
        ubo.model = glm::rotate(glm::identity<glm::mat4>(), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //ubo.view = glm::translate(quatToMat(lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f))) ,glm::vec3(2.0f, 2.0f, 2.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapChain->GetExtent().width / (float) swapChain->GetExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        glm::inverse(ubo.model);

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

                // Bind the VulkanMesh
                roomMesh->Bind(commandBuffers[imageIndex]);
                // Main Draw command
                roomMesh->Draw(commandBuffers[imageIndex]);
            }
            renderPass->End(commandBuffers[imageIndex]);
        }
        commandBuffers[imageIndex]->End();
    }

    void drawFrame() {
        swapChain->WaitForLastSubmit();
        int imageIndex = swapChain->AcquireNextImage();
        commandBuffers[imageIndex]->Reset();

        updateUniformBuffer(imageIndex);
        recordCommandBuffers(imageIndex);

        if (swapChain->IsInvalid() || window->IsWindowResized(true)) {
            recreateSwapChain();
            return;
        }

        // Submit
        swapChain->SubmitCommands(commandBuffers[imageIndex]);

        // Present
        swapChain->Present();

        if (swapChain->IsInvalid() || window->IsWindowResized(true)) {
            recreateSwapChain();
        }
    }
};

glm::mat4 rotateX(float rad) { return glm::mat4(1, 0, 0, 0, 0, cos(rad), sin(rad), 0, 0, -sin(rad), cos(rad), 0, 0, 0, 0, 1); }
glm::mat4 rotateY(float rad) { return glm::mat4(cos(rad), 0, -sin(rad), 0, 0, 1, 0, 0, sin(rad), 0, cos(rad), 0, 0, 0, 0, 1); }
glm::mat4 rotateZ(float rad) { return glm::mat4(cos(rad), -sin(rad), 0, 0, sin(rad), cos(rad), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1); }

void printMatrix(glm::mat4 m) {
    printf("%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n\n",
           m[0][0], m[1][0], m[2][0], m[3][0],
           m[0][1], m[1][1], m[2][1], m[3][1],
           m[0][2], m[1][2], m[2][2], m[3][2],
           m[0][3], m[1][3], m[2][3], m[3][3]);
}

glm::mat4 quatToMat(glm::vec4 q) {
    return glm::mat4(1 - 2 * q.y * q.y - 2 * q.z * q.z, 2 * q.x * q.y - 2 * q.z * q.w, 2 * q.x * q.z + 2 * q.y * q.w, 0,
                     2 * q.x * q.y + 2 * q.z * q.w, 1 - 2 * q.x * q.x - 2 * q.z * q.z, 2 * q.y * q.z - 2 * q.x * q.w, 0,
                     2 * q.x * q.z - 2 * q.y * q.w, 2 * q.y * q.z + 2 * q.x * q.w, 1 - 2 * q.x * q.x - 2 * q.y * q.y, 0,
                     0, 0, 0, 1);
}

glm::vec4 angleAxisToQuat(float angle, glm::vec3 axis) {
    glm::vec4 q;
    q.x = axis.x * sin(angle / 2);
    q.y = axis.y * sin(angle / 2);
    q.z = axis.z * sin(angle / 2);
    q.w = cos(angle / 2);
    return q;
}

const glm::vec3 up(0, 1, 0);
const glm::vec3 forward_(0, 0, 1);

glm::vec4 lookAt(glm::vec3 eye, glm::vec3 center) {
    const float CY_FP_EPSILON = 0.00001;
    const float M_PI = 3.1415926535897932f;
    const auto &forward = glm::normalize(center - eye);
    const float dot = glm::dot(forward_, forward);

    if (abs(dot - (-1.0f)) < CY_FP_EPSILON)
        return glm::vec4(up.x, up.y, up.z, M_PI);

    if (abs(dot - 1.0f) < CY_FP_EPSILON)
        return glm::vec4(0, 0, 0, 1);

    const float angle = acos(dot);
    const auto &axis = glm::normalize(glm::cross(forward_, forward));

    return angleAxisToQuat(angle, axis);
}

int main(int argc, char **argv) {

    auto m1 = glm::lookAt(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(4.0f, 5.0f, 6.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    printMatrix(m1);

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