//
// Created by alberto on 4/8/26.
//

#include "../../../include/Engine/Core/Vulkan/VInstanceManager.h"

#include <Engine/Utils/TypeAliases.h>

#include <assert.h>
#include <iostream>
#include <ostream>

VInstanceManager::VInstanceManager(const char* p_appName, const char* p_engineName, const uint32_t p_appVersion,
                                   const uint32_t p_engineVersion) noexcept
{
    m_vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    m_vkApplicationInfo.pNext = nullptr;
    m_vkApplicationInfo.pApplicationName = p_appName;
    m_vkApplicationInfo.applicationVersion = p_appVersion;
    m_vkApplicationInfo.pEngineName = p_engineName;
    m_vkApplicationInfo.engineVersion = p_engineVersion;
    m_vkApplicationInfo.apiVersion = VK_MAKE_API_VERSION(0,1,0,0);

    fillInstanceInfo();
    createInstance();
}

void VInstanceManager::fillInstanceInfo() noexcept
{
    m_vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    m_vkInstanceCreateInfo.pNext = nullptr;
    m_vkInstanceCreateInfo.pApplicationInfo = &m_vkApplicationInfo;
    m_vkInstanceCreateInfo.flags = 0;

    //checkLayer();

    uint32_t glfwCountExtension {0};
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwCountExtension);
    std::cout << glfwCountExtension << " extensions enabled for GLFW.\n";
    m_vkInstanceCreateInfo.enabledExtensionCount = { glfwCountExtension };
    m_vkInstanceCreateInfo.ppEnabledExtensionNames = { glfwExtensions };
}

void VInstanceManager::createInstance() noexcept
{
    auto result = vkCreateInstance(&m_vkInstanceCreateInfo, nullptr, &m_vkInstance);
    assert (result == VK_SUCCESS);
    std::cout << "VkInstance created succesfully" << std::endl;
}

void VInstanceManager::checkLayer() noexcept
{
    uint32_t layerCount { 0 };
    vkEnumerateInstanceLayerProperties( &layerCount, nullptr);
    Vector<VkLayerProperties> layers( layerCount );
    vkEnumerateInstanceLayerProperties( &layerCount, layers.data());

    const Vector<const char*> _validationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };

    bool layerFound { false };
    for(auto& neededLayer : _validationLayers)
    {
        layerFound = false;
        for(auto& availibleLayer : layers)
        {
            if( std::strcmp( neededLayer, availibleLayer.layerName ) == 0 )
            {
                std::cout << "Layer " << neededLayer << " in use ;D" << std::endl;
                layerFound = true;
                break;
            }
        }

        if( !layerFound )
        {
            std::cout << "Layer named : " << neededLayer << " is not suported." << std::endl;
            assert( layerFound );
        }
    }

    m_vkInstanceCreateInfo.enabledLayerCount = { (uint32_t)_validationLayers.size() };
    m_vkInstanceCreateInfo.ppEnabledLayerNames = { _validationLayers.data() };
}

VInstanceManager::~VInstanceManager() noexcept
{
    vkDestroyInstance(m_vkInstance, nullptr);
}
