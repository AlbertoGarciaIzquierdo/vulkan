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

VInstanceManager::VInstanceManager(GLFWwindow* window)
    : m_window(window)
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
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


    m_vkInstance = vk::raii::Instance(m_context, createInfo);
}

std::vector<char const*> VInstanceManager::getRequiredInstanceExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    Vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (Engine::Defaults::enableValidationLayers)
    {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
}

std::vector<char const*> VInstanceManager::getRequiredInstanceLayers() const
{
    // Get the required layers
    Vector<char const*> requiredLayers;
    if (Engine::Defaults::enableValidationLayers)
    {
        requiredLayers.assign(Engine::Defaults::validationLayers.begin(), Engine::Defaults::validationLayers.end());
    }

    return requiredLayers;
}

void VInstanceManager::validateLayers(const std::vector<char const*>& layers) const
{
    // Check if the required layers are supported by the Vulkan implementation.
    auto layerProperties = m_context.enumerateInstanceLayerProperties();
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
    auto extensionProperties = m_context.enumerateInstanceExtensionProperties();
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
    if (!Engine::Defaults::enableValidationLayers) return;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{.messageSeverity = severityFlags,
                                                                          .messageType     = messageTypeFlags,
                                                                          .pfnUserCallback = &debugCallback};
    m_debugMessenger = m_vkInstance.createDebugUtilsMessengerEXT( debugUtilsMessengerCreateInfoEXT );
}

void VInstanceManager::createSurface()
{
    VkSurfaceKHR       _surface;
    if (glfwCreateWindowSurface(*m_vkInstance, m_window, nullptr, &_surface) != 0) {
        throw std::runtime_error("failed to create window surface!");
    }
    m_surface = vk::raii::SurfaceKHR(m_vkInstance, _surface);
}

void VInstanceManager::pickPhysicalDevice()
{
    auto physicalDevices = m_vkInstance.enumeratePhysicalDevices();
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
        m_physicalDevice = candidates.rbegin()->second;
        Logger::Log(LogLevel::Debug, "> [{}] Selected", m_physicalDevice.getProperties().deviceName.data());
    }
    else
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void VInstanceManager::createLogicalDevice()
{
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    uint32_t queueIndex = ~0;
    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
    {
      if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
          m_physicalDevice.getSurfaceSupportKHR(qfpIndex, *m_surface))
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
                                               .enabledExtensionCount   = static_cast<uint32_t>(m_vrequiredDeviceExtension.size()),
                                               .ppEnabledExtensionNames = m_vrequiredDeviceExtension.data()};

    m_device = vk::raii::Device( m_physicalDevice, deviceCreateInfo );
    m_graphicsQueue  = vk::raii::Queue(m_device, queueIndex, 0);
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
    std::ranges::all_of( m_vrequiredDeviceExtension,
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
