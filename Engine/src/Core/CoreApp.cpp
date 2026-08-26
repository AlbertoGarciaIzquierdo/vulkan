//
// Created by alberto on 4/8/26.
//

#include <Engine/Core/CoreApp.h>

#include <iostream>
#include <ostream>

#include <Engine/Core/Vulkan/VInstanceManager.h>
#include <Engine/Defaults/DefaultConfig.h>

CoreApp::~CoreApp()
{
    cleanup();
    Logger::Shutdown();
}

void CoreApp::Run()
{
    Logger::Init();

    try
    {
        m_instanceManager = std::make_unique<VInstanceManager>();
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
}

void CoreApp::cleanup()
{
    if (m_instanceManager)
    {
        m_instanceManager.reset();
    }
}
