#pragma once

#include "lve_model.hpp"
#include <memory>

struct Transform2dComponent
{
    glm::vec2 translation{};
    glm::vec2 scale{1.0f, 1.0f};
    float rotation{0.0f};

    glm::mat2 mat2() {
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);

        glm::mat2 rotMatrix{{c, s}, {-s, c}};
        glm::mat2 scaleMat{{scale.x, 0.0f}, {0.0f, scale.y}};

        return rotMatrix * scaleMat;
    }

};


namespace lve {
    class LveGameObject {
        public:
        using id_t = unsigned int;

        static LveGameObject createLveGameObject () {
            static id_t currentId = 0;
            return LveGameObject{currentId++};
        }

        LveGameObject(const LveGameObject&) = delete;
        LveGameObject &operator=(const LveGameObject&) = delete;
        LveGameObject(LveGameObject&&) = default;
        LveGameObject &operator=(LveGameObject&&) = default;

        const id_t id;
        std::shared_ptr<LveModel> model;
        glm::vec3 color;
        Transform2dComponent translation2d;

        private:
        LveGameObject(id_t objId) : id{objId} {}

    };
}