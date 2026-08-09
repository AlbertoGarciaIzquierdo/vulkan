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
    ~CoreApp() = default;
    void Run() override;
private:
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    GLFWwindow* window = nullptr;
    UniqPtr<VInstanceManager> m_instanceManager = nullptr;
};
