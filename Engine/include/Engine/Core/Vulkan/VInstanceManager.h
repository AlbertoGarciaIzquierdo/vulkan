//
// Created by alberto on 4/8/26.
//

#pragma once

#include <Engine/Utils/GraphicIncludes.h>

class VInstanceManager
{
public:
    const vk::raii::Instance& getVkInstance() { return m_vkInstance; };
private:
    vk::raii::Context  context;
    vk::raii::Instance m_vkInstance = nullptr;

    void fillInstanceInfo();
    void createInstance();
    void checkLayer();
public:
    VInstanceManager(const char* p_appName = "App", const char* p_engineName = "Engine");
    ~VInstanceManager();
};
