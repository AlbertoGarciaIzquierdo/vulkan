//
// Created by alberto on 4/8/26.
//

#pragma once

#include <utils/GraphicIncludes.h>

class VPhysicalDeviceManager
{
private:
    VkInstance m_vkInstance;
    bool checkDeviceSuitability(VkPhysicalDevice p_device);
public:
    VPhysicalDeviceManager(const VkInstance& vkInstance);
    ~VPhysicalDeviceManager() = default;
};
