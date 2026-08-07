//
// Created by alberto on 7/8/26.
//

#pragma once
#include <Engine/Utils/TypeAliases.h>

namespace Engine
{
    namespace Defaults
    {
        constexpr UInt32 WIDTH = 800;
        constexpr UInt32 HEIGHT = 600;
        constexpr String APP_NAME = "App";
        constexpr String ENGINE_NAME = "Engine";

        const std::vector<char const*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

#ifdef NDEBUG
        constexpr bool enableValidationLayers = false;
        constexpr bool DEBUG_MODE = false;
#else
        constexpr bool enableValidationLayers = true;
        constexpr bool DEBUG_MODE = true;
#endif
    }
}
