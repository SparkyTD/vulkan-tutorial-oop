#pragma once

#include <Windows.h>
#include "vk_common.h"

class VkValidationClient {
public:
    void Connect();
    void Disconnect();

    void SendDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                          const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);

private:
    HANDLE hPipe;
};
