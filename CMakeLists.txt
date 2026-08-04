cmake_minimum_required(VERSION 3.20)
project(SharpEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(include)
add_subdirectory(src)

add_executable(SharpEngine
        src/main.cpp
        src/core/CoreApp.cpp
        src/core/vulkan/VInstanceManager.cpp
        src/core/vulkan/VPhysicalDeviceManager.cpp)

target_include_directories(SharpEngine
        PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(SharpEngine
        PRIVATE
        glfw
        Vulkan::Vulkan
)