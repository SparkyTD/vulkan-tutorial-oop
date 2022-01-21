#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION

#include "tiny_obj_loader.h"

#include "VulkanBuffer.h"

Mesh::Mesh(const char *path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path)) {
        throw std::runtime_error(err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto &shape: shapes) {
        for (const auto &index: shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

std::shared_ptr<VulkanBuffer>
Mesh::CreateIndexBuffer(std::shared_ptr<VulkanCommandPool> commandPool, std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanDevice> device) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    auto stagingBuffer = std::make_shared<VulkanBuffer>(device, instance, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto indexBuffer = std::make_shared<VulkanBuffer>(device, instance, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    stagingBuffer->CopyFrom(indices.data(), bufferSize);
    stagingBuffer->CopyTo(commandPool, indexBuffer, bufferSize);

    return indexBuffer;
}

std::shared_ptr<VulkanBuffer>
Mesh::CreateVertexBuffer(std::shared_ptr<VulkanCommandPool> commandPool, std::shared_ptr<VulkanInstance> instance, std::shared_ptr<VulkanDevice> device) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    auto stagingBuffer = std::make_shared<VulkanBuffer>(device, instance, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto vertexBuffer = std::make_shared<VulkanBuffer>(device, instance, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    stagingBuffer->CopyFrom(vertices.data(), bufferSize);
    stagingBuffer->CopyTo(commandPool, vertexBuffer, bufferSize);

    return vertexBuffer;
}

uint32_t Mesh::GetIndexCount() const {
    return indices.size();
}

uint32_t Mesh::GetVertexCount() const {
    return vertices.size();
}

