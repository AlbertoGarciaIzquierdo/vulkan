//
// Created by alberto on 7/8/26.
//

#pragma once
#include <Engine/Utils/TypeAliases.h>

/** @brief Editor-specific default settings. */
namespace Editor
{
    namespace Defaults
    {
        constexpr UInt32 WIDTH = 800;   ///< Initial window width in pixels.
        constexpr UInt32 HEIGHT = 600;  ///< Initial window height in pixels.

        /** Directory with the editor shaders (injected by CMake as EDITOR_SHADER_DIR). */
        const inline std::filesystem::path SHADER_PATH{
            EDITOR_SHADER_DIR
        };
    }
}
