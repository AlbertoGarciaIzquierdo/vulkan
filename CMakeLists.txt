cmake_minimum_required(VERSION 3.20)

project(SharpEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(SLANGC_EXECUTABLE slangc)

if (NOT SLANGC_EXECUTABLE)
    message(FATAL_ERROR
            "slangc was not found. "
            "Install Slang or the Vulkan SDK and add its bin directory to PATH."
    )
endif()

message(STATUS "Using slangc: ${SLANGC_EXECUTABLE}")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompileShaders.cmake")

add_subdirectory(Engine)
add_subdirectory(Editor)