#pragma once

#include "vk_common.h"

class Mesh {
    VK_NON_COPIABLE(Mesh)

public:
    Mesh(const char *path);

    std::shared_ptr<VulkanBuffer> CreateIndexBuffer(std::shared_ptr<VulkanCommandPool> commandPool, std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanDevice> device);

    std::shared_ptr<VulkanBuffer>
    CreateVertexBuffer(std::shared_ptr<VulkanCommandPool> commandPool, std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanDevice> device);

    uint32_t GetIndexCount() const;

    uint32_t GetVertexCount() const;

private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

