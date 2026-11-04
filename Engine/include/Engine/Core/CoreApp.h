//
// Created by alberto on 4/8/26.
//

#pragma once
#include <Engine/Core/BaseApplication.h>
#include <Engine/Core/Vulkan/VInstanceManager.h>

class GLFWwindow;

/**
 * @class CoreApp
 * @brief Default engine application: starts the logger and the Vulkan instance manager.
 */
class CoreApp : public BaseApplication
{
public:
    CoreApp() = default;
    /** @brief Releases the Vulkan manager and shuts the logger down. */
    ~CoreApp();
    /**
     * @brief Initializes the logger and creates the VInstanceManager, which blocks in its main loop.
     *
     * Any exception thrown during initialization or rendering is caught and logged.
     */
    void Run() override;
private:
    /** @brief Destroys the VInstanceManager if it exists. */
    void cleanup();
    UniqPtr<VInstanceManager> m_instanceManager = nullptr; ///< Owns the window and the Vulkan stack.
};
