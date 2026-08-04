//
// Created by alberto on 4/8/26.
//

#include "CoreApp.h"
#include <utils/TypeAliases.h>

#include <iostream>
#include <ostream>

#include "vulkan/VInstanceManager.h"
#include "vulkan/VPhysicalDeviceManager.h"

void CoreApp::Run()
{
    UniqPtr<VInstanceManager> instanceManager = std::make_unique<VInstanceManager>();
    UniqPtr<VPhysicalDeviceManager> physicalDevice = std::make_unique<VPhysicalDeviceManager>(instanceManager.get()->m_vkInstance);

    std::cout << "Run CoreApp" << std::endl;
}
