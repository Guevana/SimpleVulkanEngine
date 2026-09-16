#pragma once

#include "lve_Window.hpp"
#include "lve_device.hpp"
#include "lve_GameObject.hpp"
#include "lve_renderer.hpp"
#include "lve_buffer.hpp"
#include "lve_descriptors.hpp"

#include <memory>
#include <vector>

namespace lve{
    /**
     * 统一缓冲区数据结构
     */
    struct GlobalUbo {
        glm::mat4 projectionView{1.0f};
        glm::vec3 lightDirection = glm::normalize(glm::vec3{1.0f, -3.0f, -1.0f});
    };
    
    class FirstAPP{
        public:

        static constexpr int Width = 800;
        static constexpr int Height = 600;

        FirstAPP();
        ~FirstAPP();

        FirstAPP(const FirstAPP &) = delete;
        FirstAPP &operator=(const FirstAPP &) =delete;

        void run();
     
        private:
        void loadGameObjects();
        

        LveWindow lveWindow{Width, Height, "Hello Vulkan!"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};
        
        std::unique_ptr<LveDescriptorPool> globalPool{};
        std::vector<LveGameObject> gameObjects;
    };

}