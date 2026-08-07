//
// Created by alberto on 4/8/26.
//

#pragma once

#include <Engine/Utils/GraphicIncludes.h>

class VDeviceManager
{
private:
    VkInstance m_vkInstance;
    bool checkDeviceSuitability(VkPhysicalDevice p_device);
public:
    VDeviceManager(const vk::Instance& vkInstance);
    ~VDeviceManager() = default;
};
