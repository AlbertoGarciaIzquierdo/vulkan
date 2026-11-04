## Linux build dependencies.
## Uncomment and run the command for your distribution.

# Fedora
sudo dnf install \
    clang \
    lld \
    doxygen \
    vulkan-tools \
    vulkan-loader-devel \
    vulkan-validation-layers \
    SDL3-devel \
    glfw-devel \
    glslc \
    libXi-devel \
    libXxf86vm-devel

# Debian / Ubuntu Note: This is not tested.
# sudo apt install \
#    clang \
#    lld \
#    doxygen \
#    vulkan-tools \
#    libvulkan-dev \
#    vulkan-validationlayers \
#    libglfw3-dev \
#    libsdl3-dev \
#    glslc \
#    libxi-dev \
#    libxxf86vm-dev

# Arch Linux Note: This is not tested.
# sudo pacman -S --needed \
#     clang \
#     lld \
#     doxygen \
#     vulkan-tools \
#     vulkan-headers \
#     vulkan-icd-loader \
#     vulkan-validation-layers \
#     glfw \
#     SDL3-devel \
#     shaderc \
#     libxi \
#     libxxf86vm

# Windows dependencies are documented in README.md.
