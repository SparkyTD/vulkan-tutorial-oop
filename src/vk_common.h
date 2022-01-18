#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <memory>

#include "vk_forward.h"
#include "vk_structures.h"

// Vulkan object handle pattern
#define VK_HANDLE(type, name) \
private: \
    type name{};              \
public:  \
    type Handle() const {return name;}

// Non-copiable constructor
#define VK_NON_COPIABLE(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName(ClassName&&) = delete; \
    ClassName& operator= (const ClassName&) = delete; \
    ClassName& operator= (ClassName&&) = delete;

template<class TDevice, class THandle, class TAllocator>
inline void VkDestroy(void(vkDestroy)(TDevice, THandle, TAllocator), TDevice device, THandle &handle) {
    if (handle != VK_NULL_HANDLE) {
        vkDestroy(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}

template<class THandle, class TAllocator>
inline void VkDestroy(void(vkDestroy)(THandle, TAllocator), THandle &handle) {
    if (handle != VK_NULL_HANDLE) {
        vkDestroy(handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}

template<class THandleSource1, class THandleSource2, class THandleElem>
inline std::vector<THandleElem> VkEnumerateVector(THandleSource1 sourceHandle1, THandleSource2 sourceHandle2,
                                                  VkResult(enumerator)(THandleSource1, THandleSource2, uint32_t *, THandleElem *)) {
    uint32_t size = 0;
    enumerator(sourceHandle1, sourceHandle2, &size, nullptr);

    std::vector<THandleElem> vector(size);
    enumerator(sourceHandle1, sourceHandle2, &size, vector.data());

    return vector;
}

template<class THandleSource1, class THandleSource2, class THandleElem>
inline std::vector<THandleElem> VkEnumerateVector(THandleSource1 sourceHandle1, nullptr_t,
                                                  VkResult(enumerator)(THandleSource1, THandleSource2, uint32_t *, THandleElem *)) {
    uint32_t size = 0;
    enumerator(sourceHandle1, nullptr, &size, nullptr);

    std::vector<THandleElem> vector(size);
    enumerator(sourceHandle1, nullptr, &size, vector.data());

    return vector;
}

template<class THandleSource1, class THandleElem>
inline std::vector<THandleElem> VkEnumerateVector(THandleSource1 sourceHandle1, VkResult(enumerator)(THandleSource1, uint32_t *, THandleElem *)) {
    uint32_t size = 0;
    enumerator(sourceHandle1, &size, nullptr);

    std::vector<THandleElem> vector(size);
    enumerator(sourceHandle1, &size, vector.data());

    return vector;
}

template<class THandleSource1, class THandleElem>
inline std::vector<THandleElem> VkEnumerateVector(THandleSource1 sourceHandle1, void(enumerator)(THandleSource1, uint32_t *, THandleElem *)) {
    uint32_t size = 0;
    enumerator(sourceHandle1, &size, nullptr);

    std::vector<THandleElem> vector(size);
    enumerator(sourceHandle1, &size, vector.data());

    return vector;
}

template<class THandleSource1, class THandleElem>
inline std::vector<THandleElem> VkEnumerateVector(nullptr_t, VkResult(enumerator)(THandleSource1, uint32_t *, THandleElem *)) {
    uint32_t size = 0;
    enumerator(nullptr, &size, nullptr);

    std::vector<THandleElem> vector(size);
    enumerator(nullptr, &size, vector.data());

    return vector;
}