//
// Created by alberto on 4/8/26.
//

#include <Engine/Core/CoreApp.h>
#include <Engine/Utils/TypeAliases.h>

#include <iostream>
#include <ostream>

#include <Engine/Core/Vulkan/VInstanceManager.h>
#include "../../include/Engine/Core/Vulkan/VPhysicalDeviceManager.h"

void CoreApp::Run()
{
    UniqPtr<VInstanceManager> instanceManager = std::make_unique<VInstanceManager>();
    UniqPtr<VPhysicalDeviceManager> physicalDevice = std::make_unique<VPhysicalDeviceManager>(instanceManager.get()->m_vkInstance);

    std::cout << "Run CoreApp" << std::endl;
}
