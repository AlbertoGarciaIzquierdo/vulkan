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
    }
    catch (const EngineException& exception) {
        Logger::Log(LogLevel::Error,"{}", exception.what());
    }
    catch (const std::exception& e)
    {
        Logger::Log(LogLevel::Error, "{}", e.what());
    }
    catch (...)
    {
        Logger::Log(LogLevel::Critical, "Unknown error");
    }

    // Pase lo que pase, intentamos limpiar
    cleanup();
    Logger::Shutdown();
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

    m_instanceManager = std::make_unique<VInstanceManager>(window);
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
    if (m_instanceManager)
    {
        m_instanceManager.reset();
    }

    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}
