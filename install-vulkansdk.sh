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

        return 1 2>/dev/null || exit 1
    fi

    mkdir -p "$SDK_ROOT" || {
        echo "Failed to create SDK directory."
        return 1 2>/dev/null || exit 1
    }

    echo "Extracting:"
    echo "  $archive"

    if ! tar -xf "$archive" -C "$SDK_ROOT"; then
        echo "Failed to extract the Vulkan SDK."
        return 1 2>/dev/null || exit 1
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
        return 1 2>/dev/null || exit 1
    fi
fi

echo "Loading Vulkan environment:"
echo "  $setup_env"

. "$setup_env"

if [ -z "${VULKAN_SDK:-}" ]; then
    echo "VULKAN_SDK was not defined by setup-env.sh."
    return 1 2>/dev/null || exit 1
fi

export VK_SDK_PATH="$VULKAN_SDK"

case ":${PATH:-}:" in
    *":$VULKAN_SDK/bin:"*)
        ;;
    *)
        export PATH="$VULKAN_SDK/bin:$PATH"
        ;;
esac

echo
echo "Vulkan SDK configured:"
echo "  VULKAN_SDK=$VULKAN_SDK"
echo "  VK_SDK_PATH=$VK_SDK_PATH"

install_link() {
    executable_name="$1"
    source_path="$VULKAN_SDK/bin/$executable_name"
    link_path="/usr/local/bin/$executable_name"

    if [ ! -x "$source_path" ]; then
        echo "$executable_name: not found in $VULKAN_SDK/bin"
        return 0
    fi

    current_target=""

    if [ -L "$link_path" ]; then
        current_target=$(readlink -f "$link_path" 2>/dev/null || true)
    fi

    resolved_source=$(readlink -f "$source_path" 2>/dev/null || true)

    if [ "$current_target" = "$resolved_source" ]; then
        echo "$executable_name link is already correct."
        return 0
    fi

    echo "Creating link:"
    echo "  $link_path -> $source_path"

    sudo install -d -m 0755 /usr/local/bin || return 1

    sudo ln -sfn \
        "$source_path" \
        "$link_path"
}

install_link slangc
install_link slangd

hash -r 2>/dev/null || true

echo
command -v vulkaninfo || echo "vulkaninfo: not found"
command -v slangc || echo "slangc: not found"
command -v slangd || echo "slangd: not found"