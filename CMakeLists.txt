cmake_minimum_required(VERSION 3.20)
project(SharpEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(SharpEngine)

target_include_directories(SharpEngine
        PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(SharpEngine
        PRIVATE
        glfw
        Vulkan::Vulkan
)

add_subdirectory(include)
add_subdirectory(src)