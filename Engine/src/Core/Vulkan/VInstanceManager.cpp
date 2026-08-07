//
// Created by alberto on 4/8/26.
//

#include <Engine/Core/Vulkan/VInstanceManager.h>

#include <Engine/Utils/TypeAliases.h>

#include <assert.h>
#include <iostream>
#include <ostream>

#include "Engine/Defaults/DefaultConfig.h"

VInstanceManager::VInstanceManager(const char* p_appName, const char* p_engineName)
{
    fillInstanceInfo();
    createInstance();

    constexpr vk::ApplicationInfo appInfo{.pApplicationName   = Engine::Defaults::APP_NAME,
                                      .applicationVersion = VK_MAKE_VERSION(0,1,0),
                                      .pEngineName        = Engine::Defaults::ENGINE_NAME,
                                      .engineVersion      = VK_MAKE_VERSION(0,1,0),
                                      .apiVersion         = vk::ApiVersion14};

    // Get the required instance extensions from GLFW.
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Check if the required GLFW extensions are supported by the Vulkan implementation.
    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
    {
        if (std::ranges::none_of(extensionProperties,
                                 [glfwExtension = glfwExtensions[i]](auto const& extensionProperty)
                                 { return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
        {
            throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
        }
    }

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions};

    try
    {
        vk::raii::Context context;
        m_vkInstance = vk::raii::Instance(context, createInfo);
    }
    catch (const vk::SystemError& err)
    {
        std::cerr << "Vulkan error: " << err.what() << std::endl;
        return;
    }
    catch (const std::exception& err)
    {
        std::cerr << "Error: " << err.what() << std::endl;
        return;
    }
}

void VInstanceManager::fillInstanceInfo()
{

}

void VInstanceManager::createInstance()
{

}

void VInstanceManager::checkLayer()
{

}

VInstanceManager::~VInstanceManager()
{

}
