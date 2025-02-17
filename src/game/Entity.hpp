#pragma once

#include <string>
#include <glm/ext/vector_float3.hpp>
#include "Transform.hpp"

typedef unsigned int GLuint;

class Entity
{
public:
    Entity(int32_t id);
    Entity(int32_t id, const glm::vec3& position);
    Entity(int32_t id, const glm::vec3& position, const std::string& name);

    void draw() const;

private:
    void init();

public:
    int32_t id;
    std::string name;
    Transform transform, smooth_transform;

    GLuint VAO, VBO, EBO;
    int32_t indices_count;
};
