//
// Created by alberto on 31/7/26.
//

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#include "core/CoreApp.h"

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported\n";

    //while (!glfwWindowShouldClose(window)) {
    //
    //    glfwPollEvents();
    //}

    glfwDestroyWindow(window);

    glfwTerminate();


    CoreApp coreApp;
    coreApp.Run();

    return 0;
}
