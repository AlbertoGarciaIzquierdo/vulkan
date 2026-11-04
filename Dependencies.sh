## Linux build dependencies.
## Uncomment and run the command for your distribution.

# Fedora
sudo dnf install \
    vulkan-tools \
    vulkan-loader-devel \
    vulkan-validation-layers \
    glfw-devel \
    glslc \
    libXi-devel \
    libXxf86vm-devel

# Debian / Ubuntu
# sudo apt install \
#     vulkan-tools \
#     libvulkan-dev \
#     vulkan-validationlayers \
#     libglfw3-dev \
#     glslc \
#     libxi-dev \
#     libxxf86vm-dev

# Arch Linux
# sudo pacman -S --needed \
#     vulkan-tools \
#     vulkan-headers \
#     vulkan-icd-loader \
#     vulkan-validation-layers \
#     glfw \
#     shaderc \
#     libxi \
#     libxxf86vm

# Windows dependencies are documented in README.md.
