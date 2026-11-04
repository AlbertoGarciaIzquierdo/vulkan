//
// Created by alberto on 7/8/26.
//

#pragma once
#include <Engine/Utils/TypeAliases.h>

/** @brief Engine-wide default settings. */
namespace Engine
{
    namespace Defaults
    {
        constexpr UInt32 WIDTH = 800;               ///< Initial window width in pixels.
        constexpr UInt32 HEIGHT = 600;              ///< Initial window height in pixels.
        constexpr String APP_NAME = "App";          ///< Application name reported to Vulkan.
        constexpr String ENGINE_NAME = "Engine";    ///< Engine name reported to Vulkan.

        /** Validation layers enabled when ENABLE_VALIDATION_LAYERS is true. */
        const std::vector<char const*> VALIDATION_LAYERS = {
            "VK_LAYER_KHRONOS_validation"
        };

        /** Directory with the compiled shaders (injected by CMake as ENGINE_SHADER_DIR). */
        inline const std::filesystem::path SHADER_PATH{
            ENGINE_SHADER_DIR
        };
#ifdef NDEBUG
        constexpr bool ENABLE_VALIDATION_LAYERS = false;    ///< Vulkan validation layers (debug builds only).
        constexpr bool DEBUG_MODE = false;                  ///< Enables Trace/Debug logging (debug builds only).
#else
        constexpr bool ENABLE_VALIDATION_LAYERS = true;     ///< Vulkan validation layers (debug builds only).
        constexpr bool DEBUG_MODE = true;                   ///< Enables Trace/Debug logging (debug builds only).
#endif
    }
}
