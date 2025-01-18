#pragma once

#include "glad/gl.h"
#include "Transform.h"
#include <string>

class Entity
{
public:
    Entity(int id);
    Entity(int id, const glm::vec3& position);
    Entity(int id, const glm::vec3& position, const std::string& name);

    void draw() const;

private:
    void init();

public:
    int id;
    std::string name;
    Transform transform, smooth_transform;

    GLuint VAO, VBO, EBO;
    int indices_count;
};
