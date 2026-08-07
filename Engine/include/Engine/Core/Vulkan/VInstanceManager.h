//
// Created by alberto on 4/8/26.
//

#pragma once

#include <Engine/Utils/GraphicIncludes.h>
#include <Engine/Utils/TypeAliases.h>

class VInstanceManager
{
public:
    const vk::raii::Instance& getVkInstance() { return m_vkInstance; };
private:
    vk::raii::Context m_context;
    vk::raii::Instance m_vkInstance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;
    vk::PhysicalDeviceFeatures deviceFeatures;
    vk::raii::Queue graphicsQueue = nullptr;
    std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

    void createInstance();
    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();

    bool isDeviceSuitable(vk::raii::PhysicalDevice const & physicalDevice);

    Vector<char const*> getRequiredInstanceLayers() const;
    void validateLayers(const std::vector<char const*>& layers) const;
    Vector<char const*> getRequiredInstanceExtensions() const;
    void validateExtensions(const std::vector<char const*>& extensions) const;

public:
    VInstanceManager();
    ~VInstanceManager() = default;
};
