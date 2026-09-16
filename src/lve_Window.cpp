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
        glfwSetWindowFocusCallback(window, focusCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetKeyCallback(window, keyCallback);

        //设置输入模式
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void LveWindow::releaseCamera() {
        if (cameraCapture.active()) {
            cameraCapture.release();
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    void LveWindow::focusCallback(GLFWwindow* window, int focused) {
        if (!focused) {
            auto& self = *static_cast<LveWindow*>(glfwGetWindowUserPointer(window));
            self.releaseCamera();
            self.cameraCapture.cancel();
        }
    }

    void LveWindow::mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
            auto& self = *static_cast<LveWindow*>(glfwGetWindowUserPointer(window));
            self.releaseCamera();
            self.cameraCapture.buttonReleased();
        }
    }

    void LveWindow::keyCallback(GLFWwindow* window, int key, int, int action, int) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            auto& self = *static_cast<LveWindow*>(glfwGetWindowUserPointer(window));
            self.releaseCamera();
            self.cameraCapture.cancel();
        }
    }

    bool LveWindow::updateCameraCapture(bool uiWantsMouse) {
        const bool down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        const bool focused = glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
        const bool escape = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
        const bool wasCaptured = cameraCapture.active();
        const bool captured = cameraCapture.update(down, focused, escape, uiWantsMouse);
        if (captured != wasCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }
        return captured;
    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        const VkResult result = glfwCreateWindowSurface(instance, window, nullptr, surface);
        if (result != VK_SUCCESS) {
            throw std::runtime_error(
                "WindowSurface创建失败, VkResult=" + std::to_string(result));
        }
    }
}
