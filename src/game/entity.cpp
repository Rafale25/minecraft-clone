#include "entity.hpp"
#include <iostream>

#include "VAO.hpp"

Entity::Entity(int id): id(id)
{
    float vertices[] = {
        // top rect
        -0.5f,  0.5f, -0.5f,  // back left
         0.5f,  0.5f, -0.5f,  // back right
        -0.5f,  0.5f,  0.5f,  // front left
         0.5f,  0.5f,  0.5f,  // front right

        // bottom rect
        -0.5f,  -0.5f, -0.5f,  // back left
         0.5f,  -0.5f, -0.5f,  // back right
        -0.5f,  -0.5f,  0.5f,  // front left
         0.5f,  -0.5f,  0.5f,  // front right
    };

    uint indices[] = {
        // top
        0, 1, 2,
        1, 3, 2,

        // bot
        4, 6, 5,
        5, 6, 7,

        // front
        0, 4, 1,
        1, 4, 5,

        // back
        2, 3, 6,
        3, 7, 6,

        // left
        0, 2, 6,
        6, 4, 0,

        // right
        1, 7, 3,
        7, 1, 5
    };

    vertex_count = sizeof(indices) / sizeof(uint);

    VBO = createBuffer(vertices, sizeof(vertices), GL_STATIC_DRAW);
    EBO = createBuffer(indices, sizeof(indices), GL_STATIC_DRAW);
    VAO = createVAO(VBO, "3f", EBO);
}

void Entity::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, 0);
}
