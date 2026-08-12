//
// Created by alberto on 4/8/26.
//

#include <Engine/Core/Vulkan/VInstanceManager.h>

#include <Engine/Utils/TypeAliases.h>
#include "Engine/Defaults/DefaultConfig.h"

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
                                                      vk::DebugUtilsMessageTypeFlagsEXT              type,
                                                      const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
                                                      void *                                         pUserData)
{
    std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

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

VInstanceManager::VInstanceManager(GLFWwindow* window)
    : _window(window)
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createGraphicsPipeline();
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
}

void VInstanceManager::createSurface()
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(*_vkInstance, _window, nullptr, &surface) != 0)
    {
        throw std::runtime_error("failed to create window surface!");
    }
    _surface = vk::raii::SurfaceKHR(_vkInstance, surface);
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

    uint32_t queueIndex = ~0;
    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
    {
      if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
          _physicalDevice.getSurfaceSupportKHR(qfpIndex, *_surface))
      {
        queueIndex = qfpIndex;
        break;
      }
    }
    if (queueIndex == ~0)
    {
      throw std::runtime_error("Could not find a queue for graphics and present");
    }

		vk::StructureChain<vk::PhysicalDeviceFeatures2,
		                   vk::PhysicalDeviceVulkan11Features,
		                   vk::PhysicalDeviceVulkan13Features,
		                   vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		    featureChain = {
		        {},                                    // vk::PhysicalDeviceFeatures2
		        {.shaderDrawParameters = true},        // vk::PhysicalDeviceVulkan11Features
		        {.dynamicRendering = true},            // vk::PhysicalDeviceVulkan13Features
		        {.extendedDynamicState = true}         // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
		    };

    float                     queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{.queueFamilyIndex = queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority};
    vk::DeviceCreateInfo      deviceCreateInfo{.pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                                               .queueCreateInfoCount    = 1,
                                               .pQueueCreateInfos       = &deviceQueueCreateInfo,
                                               .enabledExtensionCount   = static_cast<uint32_t>(_vrequiredDeviceExtension.size()),
                                               .ppEnabledExtensionNames = _vrequiredDeviceExtension.data()};

    _device = vk::raii::Device( _physicalDevice, deviceCreateInfo );
    _graphicsQueue  = vk::raii::Queue(_device, queueIndex, 0);

    auto surfaceCapabilities = _physicalDevice.getSurfaceCapabilitiesKHR( *_surface );
    std::vector<vk::SurfaceFormatKHR> availableFormats = _physicalDevice.getSurfaceFormatsKHR( *_surface );
    std::vector<vk::PresentModeKHR> availablePresentModes = _physicalDevice.getSurfacePresentModesKHR( *_surface );
}

bool VInstanceManager::isDeviceSuitable( vk::raii::PhysicalDevice const & physicalDevice )
{
  // Check if the physicalDevice supports the Vulkan 1.3 API version
  bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

  // Check if any of the queue families support graphics operations
  auto queueFamilies    = physicalDevice.getQueueFamilyProperties();
  bool supportsGraphics = std::ranges::any_of( queueFamilies, []( auto const & qfp ) { return !!( qfp.queueFlags & vk::QueueFlagBits::eGraphics ); } );

  // Check if all required physicalDevice extensions are available
  auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
  bool supportsAllRequiredExtensions =
    std::ranges::all_of( _vrequiredDeviceExtension,
                         [&availableDeviceExtensions]( auto const & requiredDeviceExtension )
                         {
                           return std::ranges::any_of( availableDeviceExtensions,
                                                       [requiredDeviceExtension]( auto const & availableDeviceExtension )
                                                       { return strcmp( availableDeviceExtension.extensionName, requiredDeviceExtension ) == 0; } );
                         } );

  // Check if the physicalDevice supports the required features (shader draw parameters, dynamic rendering and extended dynamic state)
  auto features                 = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                                       vk::PhysicalDeviceVulkan11Features,
                                                                       vk::PhysicalDeviceVulkan13Features,
                                                                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
  bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                  features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
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
}

void VInstanceManager::createGraphicsPipeline() {
    vk::raii::ShaderModule shaderModule = createShaderModule(readFile( Engine::Defaults::SHADER_PATH/"triangle.spv"));
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule,  .pName = "vertMain" };
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{ .stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain" };
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
}

[[nodiscard]] vk::raii::ShaderModule VInstanceManager::createShaderModule(const std::vector<char>& code) const
{
    vk::ShaderModuleCreateInfo createInfo{ .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data()) };
    vk::raii::ShaderModule shaderModule{ _device, createInfo };
    return shaderModule;
}