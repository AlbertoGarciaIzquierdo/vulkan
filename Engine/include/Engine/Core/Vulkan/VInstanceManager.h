//
// Created by alberto on 4/8/26.
//

#pragma once

#include <Engine/Utils/GraphicIncludes.h>
#include <Engine/Utils/TypeAliases.h>

/**
 * @file VInstanceManager.h
 * @brief Owns the SDL3 window and every Vulkan object needed to render a frame.
 */

/**
 * @class VInstanceManager
 * @brief Creates and manages the SDL3 window and the Vulkan rendering stack.
 *
 * The constructor builds the whole stack in order (window, instance, debug
 * messenger, surface, device, swapchain, pipeline, command buffers and
 * synchronization objects) and then blocks in the main loop until the window
 * is closed. All Vulkan handles use the RAII wrappers from `vulkan_raii.hpp`,
 * so they are released automatically when the manager is destroyed.
 */
class VInstanceManager
{
public:
    /** @brief Returns the Vulkan instance owned by this manager. */
    const vk::raii::Instance& getVkInstance() { return _vkInstance; };

    // SDL3 related functions
    /** @brief Initializes SDL's video subsystem and creates the resizable Vulkan window. */
    void initWindow();
    /** @brief Polls SDL events and draws frames until a quit event arrives, then waits for the GPU to be idle. */
    void mainLoop();
    /** @brief Destroys the SDL window and shuts SDL down. */
    void cleanup();
    /** @brief Releases the swapchain and its image views so they can be recreated. */
    void cleanupSwapChain();

    /** @brief Set when the window's pixel size changed and the swapchain must be recreated. */
    bool framebufferResized = false;
private:
    SDL_Window* _window = nullptr;                                   ///< SDL window that backs the surface.
    vk::raii::Context _context;                                      ///< Loads the Vulkan loader and global functions.
    vk::raii::Instance _vkInstance = nullptr;                        ///< Vulkan instance.
    vk::raii::DebugUtilsMessengerEXT _debugMessenger = nullptr;      ///< Forwards validation layer messages to the Logger.
    vk::raii::SurfaceKHR _surface = nullptr;                         ///< Presentation surface created from the SDL window.
    vk::raii::PhysicalDevice _physicalDevice = nullptr;              ///< GPU selected by pickPhysicalDevice().
    vk::raii::Device _device = nullptr;                              ///< Logical device.
    vk::PhysicalDeviceFeatures _deviceFeatures;                      ///< Basic device features (currently unused).
    vk::raii::Queue _graphicsQueue = nullptr;                        ///< Queue used for both graphics and presentation.
    vk::raii::SwapchainKHR           _swapChain      = nullptr;      ///< Swapchain that holds the presentable images.
    std::vector<vk::Image>           _swapChainImages;               ///< Images owned by the swapchain.
    vk::SurfaceFormatKHR             _swapChainSurfaceFormat;        ///< Pixel format and color space of the swapchain.
    vk::Extent2D                     _swapChainExtent;               ///< Swapchain resolution in pixels.
    std::vector<vk::raii::ImageView> _swapChainImageViews;           ///< One view per swapchain image.

    vk::raii::PipelineLayout _pipelineLayout = nullptr;              ///< Pipeline layout (no descriptors or push constants yet).
    vk::raii::Pipeline       _graphicsPipeline = nullptr;            ///< Graphics pipeline that draws the triangle.

    vk::raii::CommandPool _commandPool = nullptr;                    ///< Pool the frame command buffers are allocated from.
    std::vector<vk::raii::CommandBuffer> commandBuffers;             ///< One command buffer per frame in flight.

    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;      ///< Signaled when an image is acquired (per frame in flight).
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;       ///< Signaled when rendering ends, before presenting (per swapchain image).
    std::vector<vk::raii::Fence> inFlightFences;                     ///< CPU-GPU fences, one per frame in flight.

    /** Device extensions every candidate GPU must support. */
    Vector<const char*> _vrequiredDeviceExtension = {vk::KHRSwapchainExtensionName};

    /** @brief Creates the Vulkan instance with the required layers and extensions. */
    void createInstance();
    /** @brief Installs the validation-layer messenger (only when validation is enabled). */
    void setupDebugMessenger();
    /** @brief Creates the Vulkan surface from the SDL window. @throws std::runtime_error on failure. */
    void createSurface();
    /** @brief Scores the suitable GPUs (discrete first) and selects the best one. @throws std::runtime_error if none is suitable. */
    void pickPhysicalDevice();
    /** @brief Creates the logical device and its graphics/present queue, enabling dynamic rendering and synchronization2. */
    void createLogicalDevice();
    /** @brief Creates the swapchain using the best surface format, present mode and extent. */
    void createSwapChain();
    /** @brief Creates a 2D color view for every swapchain image. */
    void createImageViews();
    /** @brief Loads the compiled shader and builds the dynamic-rendering graphics pipeline. */
    void createGraphicsPipeline();
    /**
     * @brief Wraps SPIR-V bytecode in a shader module.
     * @param code Raw SPIR-V bytes.
     * @return The created shader module.
     */
    vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
    /** @brief Creates the command pool for the graphics queue family. */
    void createCommandPool();
    /** @brief Allocates one primary command buffer per frame in flight. */
    void createCommandBuffers();
    /**
     * @brief Records the commands that draw the triangle into the given swapchain image.
     * @param imageIndex Index of the acquired swapchain image.
     */
    void recordCommandBuffer(uint32_t imageIndex);
    /**
     * @brief Records an image memory barrier that changes a swapchain image's layout.
     * @param imageIndex      Index of the swapchain image.
     * @param old_layout      Current layout.
     * @param new_layout      Layout to transition to.
     * @param src_access_mask Accesses to make available before the barrier.
     * @param dst_access_mask Accesses to make visible after the barrier.
     * @param src_stage_mask  Pipeline stages to wait for.
     * @param dst_stage_mask  Pipeline stages that wait for the barrier.
     */
    void transition_image_layout(
        uint32_t                imageIndex,
        vk::ImageLayout         old_layout,
        vk::ImageLayout         new_layout,
        vk::AccessFlags2        src_access_mask,
        vk::AccessFlags2        dst_access_mask,
        vk::PipelineStageFlags2 src_stage_mask,
        vk::PipelineStageFlags2 dst_stage_mask);
    /** @brief Creates the semaphores and fences used to synchronize frames. */
    void createSyncObjects();
    /** @brief Acquires an image, records and submits the work, and presents it. */
    void drawFrame();
    /** @brief Rebuilds the swapchain and image views after a resize; waits while the window is minimized. */
    void recreateSwapChain();

    /** @brief Checks API version, queues, extensions and features of a GPU. @return True if the GPU can run the engine. */
    bool isDeviceSuitable(vk::raii::PhysicalDevice const & physicalDevice);

    // LAYERS VALIDATIONS
    /** @brief Returns the instance layers to enable (validation layers in debug builds). */
    Vector<char const*> getRequiredInstanceLayers() const;
    /** @brief Throws std::runtime_error if any layer is not supported by the implementation. */
    void validateLayers(const std::vector<char const*>& layers) const;
    // EXTENSIONS VALIDATIONS
    /** @brief Returns the instance extensions SDL needs, plus debug utils in debug builds. */
    Vector<char const*> getRequiredInstanceExtensions() const;
    /** @brief Throws std::runtime_error if any extension is not supported by the implementation. */
    void validateExtensions(const std::vector<char const*>& extensions) const;
    // SWAP CHAIN
    /** @brief Prefers B8G8R8A8 sRGB with sRGB non-linear color space, otherwise the first available format. */
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);
    /** @brief Prefers mailbox (low latency) and falls back to FIFO, which is always available. */
    vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);
    /** @brief Uses the surface's current extent, or the window pixel size clamped to the allowed range. */
    vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
    /** @brief Requests at least 3 images, clamped to the surface limits. */
    uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);

    uint32_t queueIndex = ~0;                                        ///< Graphics/present queue family index (~0 until found).
    uint32_t frameIndex = 0;                                         ///< Current frame in flight.
public:
    /** @brief Builds the whole Vulkan stack and runs the main loop until the window closes. */
    VInstanceManager();
    ~VInstanceManager();
};
