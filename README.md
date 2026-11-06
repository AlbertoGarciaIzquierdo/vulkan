# SharpEngine (Documentation of the project is in Github Pages)

SharpEngine is a small Vulkan-based 3D engine project built for learning and experimentation. The current application opens an SDL3 window and renders a triangle; the engine is still a work in progress.

## Project structure

- `Engine/` contains the reusable engine library, including the SDL3 window, Vulkan rendering setup, logging, and Slang shaders.
- `Editor/` contains the application executable that starts the engine.
- `cmake/` contains CMake helpers, including the Slang-to-SPIR-V shader compilation step.

The renderer uses Vulkan's RAII C++ bindings, a swapchain, dynamic rendering, and synchronization2. The current triangle gets its vertex data from the shader; it does not yet use a vertex buffer.

## Requirements

The checked-in CMake presets target Linux and use:

- CMake 3.25 or newer and Ninja
- Clang/Clang++ with C++26 support, and LLD
- Vulkan SDK, including Vulkan headers/loader, validation layers, and `slangc`
- SDL3 development files
- Doxygen (optional, for API documentation)

On Fedora, `Dependencies.sh` lists the system packages used by this project. Review the script and install the relevant packages for your distribution. The Vulkan SDK must be installed separately; get it from the [official LunarG download page](https://vulkan.lunarg.com/sdk/home#linux).

To load the Vulkan SDK environment on Linux, put its archive in `~/Downloads` and source the helper from the repository root:

```sh
source install-vulkansdk.sh
```

The helper configures the current shell and may ask for `sudo` to create `slangc`/`slangd` symlinks in `/usr/local/bin`.

## Configure and build

Configure and build the Debug preset:

```sh
cmake --preset debug
cmake --build --preset debug
```

For a Release build:

```sh
cmake --preset release
cmake --build --preset release
```

Both presets use Ninja and generate their build files under `build/debug` or `build/release`. The `Editor` executable is built in the matching `Editor/` subdirectory.

## API documentation

Public headers and implementation files use Doxygen-style comments. Generate the HTML documentation from the repository root with:

```sh
doxygen Doxyfile
```

Open `build/doxygen/html/index.html` to browse the generated documentation.

## Status

This is a personal learning project and is not a finished engine. Rendering features and architecture may change as the project develops.
