//
// Created by alberto on 4/8/26.
//

#include <Engine/Core/CoreApp.h>

#include <iostream>
#include <ostream>

#include <Engine/Core/Vulkan/VInstanceManager.h>
#include <Engine/Defaults/DefaultConfig.h>

void CoreApp::Run()
{
    Logger::Init();
    try
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
        Logger::Shutdown();
    }
    catch (...)
    {
        Logger::Shutdown();
    }
}

void CoreApp::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO: This line should be deleted in the future. If not, window wont be resizable
    window = glfwCreateWindow(Engine::Defaults::WIDTH, Engine::Defaults::HEIGHT, "Vulkan window", nullptr, nullptr);
}

void CoreApp::initVulkan()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    Logger::Log(LogLevel::Debug, "{} Extensions supported", extensionCount);

    UniqPtr<VInstanceManager> instanceManager = std::make_unique<VInstanceManager>();
}

void CoreApp::mainLoop()
{
    //while (!glfwWindowShouldClose(window))
    //{
    //    glfwPollEvents();
    //}
}

void CoreApp::cleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
