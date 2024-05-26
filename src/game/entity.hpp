#pragma once

#include "glad/gl.h"
#include "transform.h"

class Entity
{
public:
    Entity(int id);

    void draw();

public:
    int id;
    Transform transform;
    GLuint VAO, VBO, EBO;
    int vertex_count;
};
