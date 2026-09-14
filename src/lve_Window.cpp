#include "lve_Window.hpp"

#include <stdexcept>

namespace lve{
    LveWindow::LveWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
        initWindow();
    }
    LveWindow::~LveWindow(){
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void LveWindow::framebufferResizedCallback(GLFWwindow *window, int width, int height) {
        auto lveWindow = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
        lveWindow->framebufferResized = true;
        lveWindow->width = width;
        lveWindow->height = height;
    }

    void LveWindow::initWindow(){
        if (!glfwInit()) {
            throw std::runtime_error("failed to initialize GLFW");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        const VkResult result = glfwCreateWindowSurface(instance, window, nullptr, surface);
        if (result != VK_SUCCESS) {
            throw std::runtime_error(
                "WindowSurface创建失败, VkResult=" + std::to_string(result));
        }
    }
}