//
// Created by alberto on 4/8/26.
//

#pragma once

#include <utils/GraphicIncludes.h>

class VInstanceManager
{
public:
    VkInstance m_vkInstance {VK_NULL_HANDLE};
private:
    VkInstanceCreateInfo m_vkInstanceCreateInfo {};
    VkApplicationInfo m_vkApplicationInfo {};

    void fillInstanceInfo() noexcept;
    void createInstance() noexcept;
    void checkLayer() noexcept;
public:
    VInstanceManager(const char* p_appName = "App", const char* p_engineName = "Engine", const uint32_t p_appVersion = 1, const uint32_t p_engineVersion = 1) noexcept;
    ~VInstanceManager() noexcept;
};
