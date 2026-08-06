//
// Created by alberto on 4/8/26.
//

#include <Engine/Core/Vulkan/VPhysicalDeviceManager.h>

#include <Engine/Utils/TypeAliases.h>

VPhysicalDeviceManager::VPhysicalDeviceManager(const VkInstance& vkInstance)
    : m_vkInstance(vkInstance)
{
    uint32_t countDevices { 0 };
    Vector<VkPhysicalDevice> _devices {};
    vkEnumeratePhysicalDevices( m_vkInstance, &countDevices, nullptr );
    _devices.resize(countDevices);
    vkEnumeratePhysicalDevices( m_vkInstance, &countDevices, &_devices[0]);
    std::cout << "Num of physical devices : " << countDevices << '\n';
    VkPhysicalDeviceProperties properties {};
    for(auto& device : _devices)
    {
        if (checkDeviceSuitability(device))
        {
            vkGetPhysicalDeviceProperties( device, &properties );
            std::cout << "Name of device : " << properties.deviceName << '\n';
            std::cout << "Type of device : " << properties.deviceType << '\n';
        }
    }
}

bool VPhysicalDeviceManager::checkDeviceSuitability(VkPhysicalDevice p_device)
{
    std::cout << "<----- Checking Devices Suitability ----->\n";

    uint32_t countQueueFamilies { 0 };
    Vector<VkQueueFamilyProperties> queueFamilies {};

    vkGetPhysicalDeviceQueueFamilyProperties( p_device, &countQueueFamilies, nullptr );
    queueFamilies.resize(countQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties( p_device, &countQueueFamilies, &queueFamilies[0] );

    for(auto& queueFamily : queueFamilies )
    {
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            return true;
    }

    return false;
}