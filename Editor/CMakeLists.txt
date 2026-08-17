add_executable(Editor
        src/main.cpp
)

target_include_directories(Editor
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Editor
        PRIVATE
        Engine
)

target_compile_definitions(
        Editor
        PUBLIC
        EDITOR_SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders"
)