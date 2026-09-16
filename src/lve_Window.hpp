#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include "camera_capture.hpp"


namespace lve{

    class LveWindow{
        public:
        LveWindow(int w,int h,std::string name);
        ~LveWindow();

        LveWindow(const LveWindow &) = delete;
        LveWindow &operator=(const LveWindow &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(window); }

        VkExtent2D getExtent() {return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
        bool wasWindowResized() {return framebufferResized; }
        void resetWindowResizedFlag() {framebufferResized = false; }
        GLFWwindow *getGLFWwindow() const {return window; }
        bool updateCameraCapture(bool uiWantsMouse);
        bool isCameraCaptured() const { return cameraCapture.active(); }
        unsigned int captureRevision() const { return cameraCapture.revision(); }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

        private:
        static void framebufferResizedCallback(GLFWwindow *window, int width, int height);
        static void focusCallback(GLFWwindow *window, int focused);
        static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
        static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        void releaseCamera();
        void initWindow();

        int width;
        int height;
        bool framebufferResized = false;
        CameraCapture cameraCapture;

        std::string windowName;

        GLFWwindow *window;
    };
}
