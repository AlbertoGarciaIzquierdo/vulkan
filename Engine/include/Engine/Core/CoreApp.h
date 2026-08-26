//
// Created by alberto on 4/8/26.
//

#pragma once
#include <Engine/Core/BaseApplication.h>
#include <Engine/Core/Vulkan/VInstanceManager.h>

class GLFWwindow;

class CoreApp : public BaseApplication
{
public:
    CoreApp() = default;
    ~CoreApp();
    void Run() override;
private:
    void cleanup();
    UniqPtr<VInstanceManager> m_instanceManager = nullptr;
};
