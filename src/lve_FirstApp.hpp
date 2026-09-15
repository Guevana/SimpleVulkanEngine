#pragma once

#include "lve_Window.hpp"
#include "lve_device.hpp"
#include "lve_GameObject.hpp"
#include "lve_renderer.hpp"

#include <memory>
#include <vector>

namespace lve{
   

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
        
        std::vector<LveGameObject> gameObjects;
    };

}