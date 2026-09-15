#pragma once

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve {

    class LveCamera {
    public:
    //正交投影矩阵
    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

    //透视投影矩阵
    void setPerspectiveProjection(float fovy, float aspect, float near, float far);

    void setViewDirection(
        glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
        
    void setViewTarget(
        glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});

    void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

    //返回设置的矩阵
    const glm::mat4& getProjection() const { return projectionMatrix; }
    const glm::mat4& getView() const { return viewMatrix; }

    private:
    glm::mat4 projectionMatrix{1.f};
    glm::mat4 viewMatrix{1.f};
    };
}  // namespace lve