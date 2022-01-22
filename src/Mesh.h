#pragma once

#include "vk_common.h"

class Mesh {
    VK_NON_COPIABLE(Mesh)

public:
    Mesh(const char *path);

    void CreateBuffers(std::shared_ptr<VulkanCommandPool> commandPool, std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanDevice> device);

    void Bind(std::shared_ptr<VulkanCommandBuffer> commandBuffer);

    void Draw(std::shared_ptr<VulkanCommandBuffer> commandBuffer);

private:
    void CreateIndexBuffer(std::shared_ptr<VulkanCommandPool> commandPool, std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanDevice> device);

    void CreateVertexBuffer(std::shared_ptr<VulkanCommandPool> commandPool, std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanDevice> device);

private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    std::shared_ptr<VulkanBuffer> vertexBuffer = nullptr;
    std::shared_ptr<VulkanBuffer> indexBuffer = nullptr;
};

