#!/bin/sh

SDK_ROOT="$HOME/VulkanSDK"
DOWNLOADS="$HOME/Downloads"

setup_env=$(
    find "$SDK_ROOT" \
        -type f \
        -name 'setup-env.sh' \
        -print \
        -quit
)

if [ -n "$setup_env" ]; then
    echo "Vulkan SDK already exists."
else
    echo "Vulkan SDK was not found. Looking for archive..."

    archive=$(
        find "$DOWNLOADS" \
            -maxdepth 1 \
            -type f \
            -name 'vulkansdk-linux-x86_64-*.tar.xz' \
            -print \
            -quit
    )

    if [ -z "$archive" ]; then
        echo "No Vulkan SDK archive was found."
        echo "Expected a file like:"
        echo "  vulkansdk-linux-x86_64-<version>.tar.xz"
        return 1
    fi

    mkdir -p "$SDK_ROOT"

    echo "Extracting:"
    echo "  $archive"

    if ! tar -xf "$archive" -C "$SDK_ROOT"; then
        echo "Failed to extract the Vulkan SDK."
        return 1
    fi

    setup_env=$(
        find "$SDK_ROOT" \
            -type f \
            -name 'setup-env.sh' \
            -print \
            -quit
    )

    if [ -z "$setup_env" ]; then
        echo "Could not find setup-env.sh after extraction."
        return 1
    fi
fi

echo "Loading Vulkan environment:"
echo "  $setup_env"

. "$setup_env"

export VK_SDK_PATH="${VULKAN_SDK:-}"

if [ -z "$VK_SDK_PATH" ]; then
    echo "VULKAN_SDK was not defined by setup-env.sh."
    return 1
fi

case ":$PATH:" in
    *":$VK_SDK_PATH/bin:"*)
        ;;
    *)
        export PATH="$VK_SDK_PATH/bin:$PATH"
        ;;
esac

echo
echo "Vulkan SDK configured:"
echo "  VULKAN_SDK=$VULKAN_SDK"
echo "  VK_SDK_PATH=$VK_SDK_PATH"

echo
command -v vulkaninfo || echo "vulkaninfo: not found"
command -v slangc || echo "slangc: not found"
command -v slangd || echo "slangd: not found"