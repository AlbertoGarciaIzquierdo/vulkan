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
    GLFWwindow* m_window = nullptr;
    vk::raii::Context m_context;
    vk::raii::Instance m_vkInstance = nullptr;
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger = nullptr;
    vk::raii::SurfaceKHR m_surface = nullptr;
    vk::raii::PhysicalDevice m_physicalDevice = nullptr;
    vk::raii::Device m_device = nullptr;
    vk::PhysicalDeviceFeatures m_deviceFeatures;
    vk::raii::Queue m_graphicsQueue = nullptr;
    Vector<const char*> m_vrequiredDeviceExtension = {vk::KHRSwapchainExtensionName};

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();

    bool isDeviceSuitable(vk::raii::PhysicalDevice const & physicalDevice);

    Vector<char const*> getRequiredInstanceLayers() const;
    void validateLayers(const std::vector<char const*>& layers) const;
    Vector<char const*> getRequiredInstanceExtensions() const;
    void validateExtensions(const std::vector<char const*>& extensions) const;

public:
    VInstanceManager(GLFWwindow* window);
    ~VInstanceManager() = default;
};
