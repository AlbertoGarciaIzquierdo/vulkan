find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_library(Engine STATIC)

target_include_directories(Engine
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Engine
        PUBLIC
        glfw
        Vulkan::Vulkan
)

set(ENGINE_SHADER_SOURCE
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.slang"
)

set(ENGINE_SHADER_OUTPUT
        "${CMAKE_CURRENT_BINARY_DIR}/shaders/triangle.spv"
)

add_slang_shader_target(
        CompileEngineShaders
        SOURCES
        "${ENGINE_SHADER_SOURCE}"
        OUTPUT
        "${ENGINE_SHADER_OUTPUT}"
)

add_dependencies(
        Engine
        CompileEngineShaders
)

target_compile_definitions(
        Engine
        PUBLIC
        ENGINE_SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders"
)

add_subdirectory(src)