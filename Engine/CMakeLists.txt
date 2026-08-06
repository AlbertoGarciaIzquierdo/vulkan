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

add_subdirectory(src)