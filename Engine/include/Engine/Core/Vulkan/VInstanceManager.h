//
// Created by alberto on 4/8/26.
//

#pragma once

#include <Engine/Utils/GraphicIncludes.h>
#include <Engine/Utils/TypeAliases.h>

class VInstanceManager
{
public:
    const vk::raii::Instance& getVkInstance() { return _vkInstance; };
private:
    GLFWwindow* _window = nullptr;
    vk::raii::Context _context;
    vk::raii::Instance _vkInstance = nullptr;
    vk::raii::DebugUtilsMessengerEXT _debugMessenger = nullptr;
    vk::raii::SurfaceKHR _surface = nullptr;
    vk::raii::PhysicalDevice _physicalDevice = nullptr;
    vk::raii::Device _device = nullptr;
    vk::PhysicalDeviceFeatures _deviceFeatures;
    vk::raii::Queue _graphicsQueue = nullptr;
    vk::raii::SwapchainKHR           _swapChain      = nullptr;
    std::vector<vk::Image>           _swapChainImages;
    vk::SurfaceFormatKHR             _swapChainSurfaceFormat;
    vk::Extent2D                     _swapChainExtent;
    std::vector<vk::raii::ImageView> _swapChainImageViews;

    vk::raii::PipelineLayout _pipelineLayout = nullptr;
    vk::raii::Pipeline       _graphicsPipeline = nullptr;

    Vector<const char*> _vrequiredDeviceExtension = {vk::KHRSwapchainExtensionName};

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createGraphicsPipeline();
    vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;

    bool isDeviceSuitable(vk::raii::PhysicalDevice const & physicalDevice);

    // LAYERS VALIDATIONS
    Vector<char const*> getRequiredInstanceLayers() const;
    void validateLayers(const std::vector<char const*>& layers) const;
    // EXTENSIONS VALIDATIONS
    Vector<char const*> getRequiredInstanceExtensions() const;
    void validateExtensions(const std::vector<char const*>& extensions) const;
    // SWAP CHAIN
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);
    vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
    uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

public:
    VInstanceManager(GLFWwindow* window);
    ~VInstanceManager() = default;
};
