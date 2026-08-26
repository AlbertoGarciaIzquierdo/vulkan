//
// Created by alberto on 4/8/26.
//

#include <Engine/Core/Vulkan/VInstanceManager.h>

#include <Engine/Utils/TypeAliases.h>
#include "Engine/Defaults/DefaultConfig.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
                                                      vk::DebugUtilsMessageTypeFlagsEXT              type,
                                                      const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
                                                      void *                                         pUserData)
{
    Logger::Log(LogLevel::Warning, "Validation layer: type {} msg: {}", to_string(type), pCallbackData->pMessage);
    //std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

    return vk::False;
}

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw EngineException(std::format(
            "Failed to open file '{}'. Working directory: '{}'",
            filename,
            std::filesystem::current_path().string()
        ));
    }

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto* app =
        static_cast<VInstanceManager*>(
            glfwGetWindowUserPointer(window)
        );

    if (app != nullptr)
    {
        app->framebufferResized = true;
    }
}

VInstanceManager::VInstanceManager()
{
    initWindow();

    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createGraphicsPipeline();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();

    mainLoop();
}

void VInstanceManager::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    _window = glfwCreateWindow(Engine::Defaults::WIDTH, Engine::Defaults::HEIGHT, "Toto Engine 2", nullptr, nullptr);
    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
}

void VInstanceManager::mainLoop()
{
    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();
        drawFrame();
    }
    _device.waitIdle();
}

void VInstanceManager::cleanup()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void VInstanceManager::cleanupSwapChain()
{
    _swapChainImageViews.clear();
    _swapChain = nullptr;
}

void VInstanceManager::createInstance()
{
    constexpr vk::ApplicationInfo appInfo{.pApplicationName   = Engine::Defaults::APP_NAME,
                                      .applicationVersion = VK_MAKE_VERSION(0,1,0),
                                      .pEngineName        = Engine::Defaults::ENGINE_NAME,
                                      .engineVersion      = VK_MAKE_VERSION(0,1,0),
                                      .apiVersion         = vk::ApiVersion14};

    // Get the required layers and validate them
    auto requiredLayers = getRequiredInstanceLayers();
    validateLayers(requiredLayers);

    // Get the required extensions and validate them
    auto requiredExtensions = getRequiredInstanceExtensions();
    validateExtensions(requiredExtensions);

    vk::InstanceCreateInfo createInfo{.pApplicationInfo        = &appInfo,
                                      .enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size()),
                                      .ppEnabledLayerNames     = requiredLayers.data(),
                                      .enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size()),
                                      .ppEnabledExtensionNames = requiredExtensions.data()};


    _vkInstance = vk::raii::Instance(_context, createInfo);
    Logger::Log(LogLevel::Debug,"Created Instance");
}

std::vector<char const*> VInstanceManager::getRequiredInstanceExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    Vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (Engine::Defaults::ENABLE_VALIDATION_LAYERS)
    {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
}

std::vector<char const*> VInstanceManager::getRequiredInstanceLayers() const
{
    // Get the required layers
    Vector<char const*> requiredLayers;
    if (Engine::Defaults::ENABLE_VALIDATION_LAYERS)
    {
        requiredLayers.assign(Engine::Defaults::VALIDATION_LAYERS.begin(), Engine::Defaults::VALIDATION_LAYERS.end());
    }

    return requiredLayers;
}

void VInstanceManager::validateLayers(const std::vector<char const*>& layers) const
{
    // Check if the required layers are supported by the Vulkan implementation.
    auto layerProperties = _context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(layers,
                                                       [&layerProperties](auto const &requiredLayer) {
                                                           return std::ranges::none_of(layerProperties,
                                                                [requiredLayer](auto const &layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
                                                       });
    if (unsupportedLayerIt != layers.end())
    {
        throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
    }
}

void VInstanceManager::validateExtensions(const std::vector<char const*>& extensions) const
{
    // Check if the required extensions are supported by the Vulkan implementation.
    auto extensionProperties = _context.enumerateInstanceExtensionProperties();
    auto unsupportedPropertyIt =
        std::ranges::find_if(extensions,
                                [&extensionProperties](auto const &requiredExtension) {
                                    return std::ranges::none_of(extensionProperties,
                                                                [requiredExtension](auto const &extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
                                });
    if (unsupportedPropertyIt != extensions.end())
    {
        throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
    }
}

void VInstanceManager::setupDebugMessenger()
{
    if (!Engine::Defaults::ENABLE_VALIDATION_LAYERS) return;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{.messageSeverity = severityFlags,
                                                                          .messageType     = messageTypeFlags,
                                                                          .pfnUserCallback = &debugCallback};
    _debugMessenger = _vkInstance.createDebugUtilsMessengerEXT( debugUtilsMessengerCreateInfoEXT );
    Logger::Log(LogLevel::Debug,"Created Debug messenger");
}

void VInstanceManager::createSurface()
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(*_vkInstance, _window, nullptr, &surface) != 0)
    {
        throw std::runtime_error("failed to create window surface!");
    }
    _surface = vk::raii::SurfaceKHR(_vkInstance, surface);
    Logger::Log(LogLevel::Debug,"Created Surface");
}

void VInstanceManager::pickPhysicalDevice()
{
    auto physicalDevices = _vkInstance.enumeratePhysicalDevices();
    if (physicalDevices.empty())
    {
        throw std::runtime_error( "failed to find GPUs with Vulkan support!" );
    }

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, vk::raii::PhysicalDevice> candidates;

    for (const auto& pd : physicalDevices)
    {
        auto deviceProperties = pd.getProperties();
        auto deviceFeatures = pd.getFeatures();
        uint32_t score = 0;

        if (!isDeviceSuitable(pd)) continue;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        {
            score += 10000;
        }

        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
        {
            score += 3000;
        }

        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
            score += 800;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader)
        {
            continue;
        }
        candidates.insert(std::make_pair(score, pd));
        Logger::Log(LogLevel::Debug, "{} [{}] SCORE: {}", pd.getProperties().deviceName.data(), to_string(pd.getProperties().deviceType), score);
    }

    // Check if the best candidate is suitable at all
    if (!candidates.empty() && candidates.rbegin()->first > 0)
    {
        _physicalDevice = candidates.rbegin()->second;
        Logger::Log(LogLevel::Debug, "> [{}] Selected", _physicalDevice.getProperties().deviceName.data());
    }
    else
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void VInstanceManager::createLogicalDevice()
{
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();

		// get the first index into queueFamilyProperties which supports both graphics and present
		for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
		{
			if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
			    _physicalDevice.getSurfaceSupportKHR(qfpIndex, *_surface))
			{
				// found a queue family that supports both graphics and present
				queueIndex = qfpIndex;
				break;
			}
		}
		if (queueIndex == ~0)
		{
			throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
		}

		// query for Vulkan 1.3 features
		vk::StructureChain<vk::PhysicalDeviceFeatures2,
		                   vk::PhysicalDeviceVulkan11Features,
		                   vk::PhysicalDeviceVulkan13Features,
		                   vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		    featureChain = {
		        {},                                                          // vk::PhysicalDeviceFeatures2
		        {.shaderDrawParameters = true},                              // vk::PhysicalDeviceVulkan11Features
		        {.synchronization2 = true, .dynamicRendering = true},        // vk::PhysicalDeviceVulkan13Features
		        {.extendedDynamicState = true}                               // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
		    };

		// create a Device
		float                     queuePriority = 0.5f;
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo{.queueFamilyIndex = queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority};
		vk::DeviceCreateInfo      deviceCreateInfo{.pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		                                           .queueCreateInfoCount    = 1,
		                                           .pQueueCreateInfos       = &deviceQueueCreateInfo,
		                                           .enabledExtensionCount   = static_cast<uint32_t>(_vrequiredDeviceExtension.size()),
		                                           .ppEnabledExtensionNames = _vrequiredDeviceExtension.data()};

		_device = vk::raii::Device(_physicalDevice, deviceCreateInfo);
		_graphicsQueue  = vk::raii::Queue(_device, queueIndex, 0);
    Logger::Log(LogLevel::Debug,"Created Logical Device");
}

bool VInstanceManager::isDeviceSuitable( vk::raii::PhysicalDevice const & physicalDevice )
{
  // Check if the physicalDevice supports the Vulkan 1.3 API version
		bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= VK_API_VERSION_1_3;

		// Check if any of the queue families support graphics operations
		auto queueFamilies    = physicalDevice.getQueueFamilyProperties();
		bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const &qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

		// Check if all required physicalDevice extensions are available
		auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
		bool supportsAllRequiredExtensions =
		    std::ranges::all_of(_vrequiredDeviceExtension,
		                        [&availableDeviceExtensions](auto const &requiredDeviceExtension) {
			                        return std::ranges::any_of(availableDeviceExtensions,
			                                                   [requiredDeviceExtension](auto const &availableDeviceExtension) { return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
		                        });

		// Check if the physicalDevice supports the required features
		auto features                 = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
		                                                                     vk::PhysicalDeviceVulkan11Features,
		                                                                     vk::PhysicalDeviceVulkan13Features,
		                                                                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
		bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
		                                features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
		                                features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
		                                features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

		// Return true if the physicalDevice meets all the criteria
		return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

vk::SurfaceFormatKHR VInstanceManager::chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats)
{
    assert(!availableFormats.empty());

    const auto formatIt = std::ranges::find_if(
        availableFormats,
        [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
    return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR VInstanceManager::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes)
{
    assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
    return std::ranges::any_of(availablePresentModes,
                               [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
               vk::PresentModeKHR::eMailbox :
               vk::PresentModeKHR::eFifo;
}

vk::Extent2D VInstanceManager::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);

    return {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

uint32_t VInstanceManager::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities)
{
    auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
    {
        minImageCount = surfaceCapabilities.maxImageCount;
    }
    return minImageCount;
}

void VInstanceManager::createSwapChain() {
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = _physicalDevice.getSurfaceCapabilitiesKHR(*_surface);
    _swapChainExtent                                = chooseSwapExtent(surfaceCapabilities);
    uint32_t minImageCount                         = chooseSwapMinImageCount(surfaceCapabilities);

    std::vector<vk::SurfaceFormatKHR> availableFormats = _physicalDevice.getSurfaceFormatsKHR(*_surface);
    _swapChainSurfaceFormat                             = chooseSwapSurfaceFormat(availableFormats);

    std::vector<vk::PresentModeKHR> availablePresentModes = _physicalDevice.getSurfacePresentModesKHR(*_surface);
    vk::PresentModeKHR              presentMode           = chooseSwapPresentMode(availablePresentModes);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{.surface          = *_surface,
                                                   .minImageCount    = minImageCount,
                                                   .imageFormat      = _swapChainSurfaceFormat.format,
                                                   .imageColorSpace  = _swapChainSurfaceFormat.colorSpace,
                                                   .imageExtent      = _swapChainExtent,
                                                   .imageArrayLayers = 1,
                                                   .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
                                                   .imageSharingMode = vk::SharingMode::eExclusive,
                                                   .preTransform     = surfaceCapabilities.currentTransform,
                                                   .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                   .presentMode      = presentMode,
                                                   .clipped          = true};

    _swapChain       = vk::raii::SwapchainKHR(_device, swapChainCreateInfo);
    _swapChainImages = _swapChain.getImages();
    Logger::Log(LogLevel::Debug,"Created Swapchain");
}

void VInstanceManager::createImageViews()
{
    assert(_swapChainImageViews.empty());

    vk::ImageViewCreateInfo imageViewCreateInfo{ .viewType         = vk::ImageViewType::e2D,
                                                 .format           = _swapChainSurfaceFormat.format,
                                                 .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };

    imageViewCreateInfo.components = {
        vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};

    imageViewCreateInfo.subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1};

    for (auto &image : _swapChainImages)
    {
        imageViewCreateInfo.image = image;
        _swapChainImageViews.emplace_back( _device, imageViewCreateInfo );
    }
    Logger::Log(LogLevel::Debug,"Created Image views and inserted");
}

void VInstanceManager::createGraphicsPipeline() {
    vk::raii::ShaderModule shaderModule = createShaderModule(readFile(Engine::Defaults::SHADER_PATH/"triangle.spv"));

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{.stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = "vertMain"};
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{.stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain"};
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    vk::PipelineVertexInputStateCreateInfo   vertexInputInfo;
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
    vk::PipelineViewportStateCreateInfo      viewportState{.viewportCount = 1, .scissorCount = 1};

    vk::PipelineRasterizationStateCreateInfo rasterizer{.depthClampEnable        = vk::False,
		                                                    .rasterizerDiscardEnable = vk::False,
		                                                    .polygonMode             = vk::PolygonMode::eFill,
		                                                    .cullMode                = vk::CullModeFlagBits::eBack,
		                                                    .frontFace               = vk::FrontFace::eClockwise,
		                                                    .depthBiasEnable         = vk::False,
		                                                    .lineWidth               = 1.0f};

    vk::PipelineMultisampleStateCreateInfo multisampling{.rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False};

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		    .blendEnable    = vk::False,
		    .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

    vk::PipelineColorBlendStateCreateInfo colorBlending{
		    .logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &colorBlendAttachment};

    std::vector<vk::DynamicState>      dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicState{.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data()};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 0, .pushConstantRangeCount = 0};
    _pipelineLayout = vk::raii::PipelineLayout(_device, pipelineLayoutInfo);

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
        {.stageCount          = 2,
         .pStages             = shaderStages,
         .pVertexInputState   = &vertexInputInfo,
         .pInputAssemblyState = &inputAssembly,
         .pViewportState      = &viewportState,
         .pRasterizationState = &rasterizer,
         .pMultisampleState   = &multisampling,
         .pColorBlendState    = &colorBlending,
         .pDynamicState       = &dynamicState,
         .layout              = _pipelineLayout,
         .renderPass          = nullptr},
        {.colorAttachmentCount = 1, .pColorAttachmentFormats = &_swapChainSurfaceFormat.format}};

    _graphicsPipeline = vk::raii::Pipeline(_device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
    Logger::Log(LogLevel::Debug,"Created Graphic pipeline");
}

[[nodiscard]] vk::raii::ShaderModule VInstanceManager::createShaderModule(const std::vector<char>& code) const
{
    vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data()) };
    vk::raii::ShaderModule shaderModule{ _device, createInfo };
    return shaderModule;
}

void VInstanceManager::createCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                       .queueFamilyIndex = queueIndex};

    _commandPool = vk::raii::CommandPool(_device, poolInfo);
}

void VInstanceManager::createCommandBuffers()
{
    vk::CommandBufferAllocateInfo allocInfo{.commandPool = _commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT};
    commandBuffers = vk::raii::CommandBuffers( _device, allocInfo );
}

void VInstanceManager::recordCommandBuffer(uint32_t imageIndex)
{
    auto &commandBuffer = commandBuffers[frameIndex];
		commandBuffer.begin({});

		// Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
		transition_image_layout(
		    imageIndex,
		    vk::ImageLayout::eUndefined,
		    vk::ImageLayout::eColorAttachmentOptimal,
		    {},                                                        // srcAccessMask (no need to wait for previous operations)
		    vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput         // dstStage
		);
		vk::ClearValue              clearColor     = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
		vk::RenderingAttachmentInfo attachmentInfo = {
		    .imageView   = _swapChainImageViews[imageIndex],
		    .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		    .loadOp      = vk::AttachmentLoadOp::eClear,
		    .storeOp     = vk::AttachmentStoreOp::eStore,
		    .clearValue  = clearColor};
		vk::RenderingInfo renderingInfo = {
		    .renderArea           = {.offset = {0, 0}, .extent = _swapChainExtent},
		    .layerCount           = 1,
		    .colorAttachmentCount = 1,
		    .pColorAttachments    = &attachmentInfo};

		commandBuffer.beginRendering(renderingInfo);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *_graphicsPipeline);
		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(_swapChainExtent.width), static_cast<float>(_swapChainExtent.height), 0.0f, 1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapChainExtent));
		commandBuffer.draw(3, 1, 0, 0);
		commandBuffer.endRendering();

		// After rendering, transition the swapchain image to vk::ImageLayout::ePresentSrcKHR
		transition_image_layout(
		    imageIndex,
		    vk::ImageLayout::eColorAttachmentOptimal,
		    vk::ImageLayout::ePresentSrcKHR,
		    vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
		    {},                                                        // dstAccessMask
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		    vk::PipelineStageFlagBits2::eBottomOfPipe                  // dstStage
		);
		commandBuffer.end();
}

void VInstanceManager::transition_image_layout(uint32_t imageIndex, vk::ImageLayout old_layout,
    vk::ImageLayout new_layout, vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask,
    vk::PipelineStageFlags2 src_stage_mask, vk::PipelineStageFlags2 dst_stage_mask)
{
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask        = src_stage_mask,
        .srcAccessMask       = src_access_mask,
        .dstStageMask        = dst_stage_mask,
        .dstAccessMask       = dst_access_mask,
        .oldLayout           = old_layout,
        .newLayout           = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = _swapChainImages[imageIndex],
        .subresourceRange    = {
            .aspectMask     = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1}};
    vk::DependencyInfo dependency_info = {
        .dependencyFlags         = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrier};
    commandBuffers[frameIndex].pipelineBarrier2(dependency_info);
}

void VInstanceManager::createSyncObjects()
{
    assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());

    for (size_t i = 0; i < _swapChainImages.size(); i++)
    {
        renderFinishedSemaphores.emplace_back(_device, vk::SemaphoreCreateInfo());
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        presentCompleteSemaphores.emplace_back(_device, vk::SemaphoreCreateInfo());
        inFlightFences.emplace_back(_device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    }
}

void VInstanceManager::drawFrame()
{
    auto fenceResult = _device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to wait for fence!");
    }

    auto [result, imageIndex] = _swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Only reset the fence if we are submitting work
    _device.resetFences(*inFlightFences[frameIndex]);

    commandBuffers[frameIndex].reset();
    recordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo   submitInfo{.waitSemaphoreCount   = 1,
                                      .pWaitSemaphores      = &*presentCompleteSemaphores[frameIndex],
                                      .pWaitDstStageMask    = &waitDestinationStageMask,
                                      .commandBufferCount   = 1,
                                      .pCommandBuffers      = &*commandBuffers[frameIndex],
                                      .signalSemaphoreCount = 1,
                                      .pSignalSemaphores    = &*renderFinishedSemaphores[imageIndex]};
    _graphicsQueue.submit(submitInfo, *inFlightFences[frameIndex]);

    const vk::PresentInfoKHR presentInfoKHR{.waitSemaphoreCount = 1,
                                            .pWaitSemaphores    = &*renderFinishedSemaphores[imageIndex],
                                            .swapchainCount     = 1,
                                            .pSwapchains        = &*_swapChain,
                                            .pImageIndices      = &imageIndex};
    result = _graphicsQueue.presentKHR(presentInfoKHR);
    if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR))
    {
        recreateSwapChain();
    }
    else
    {
        // There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
        assert(result == vk::Result::eSuccess);
    }
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VInstanceManager::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(_window, &width, &height);
        glfwWaitEvents();
    }

    _device.waitIdle();

    cleanupSwapChain();
    createSwapChain();
    createImageViews();
}
