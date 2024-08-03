#pragma once

#include "glad/gl.h"
#include "transform.h"
#include <string>

class Entity
{
public:
    Entity(int id);

    void draw();

public:
    int id;
    std::string name;
    Transform transform, smooth_transform;

    GLuint VAO, VBO, EBO;
    int indices_count;
};
