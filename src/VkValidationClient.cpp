#include "VkValidationClient.h"

#include <sstream>

void VkValidationClient::Connect() {
    hPipe = CreateFile(TEXT("\\\\.\\pipe\\VkValidationDebugger"),
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       nullptr,
                       OPEN_EXISTING,
                       0,
                       nullptr);
}

void VkValidationClient::Disconnect() {
    CloseHandle(hPipe);
}

void VkValidationClient::SendDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                          const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData) {
    int bufferLength = strlen(pCallbackData->pMessage);

    std::ostringstream oss;
    oss << messageSeverity << "\b" << messageType << "\b" << pCallbackData->pMessage;

    const std::string& resultStr = oss.str();
    const char* result = resultStr.c_str();

    DWORD dwWritten;
    WriteFile(hPipe, result, strlen(result) + 1, &dwWritten, nullptr);
}
