#include "Entity.hpp"
#include "VAO.hpp"
#include <glad/gl.h>

Entity::Entity(int32_t id): id(id) {
    init();
}

Entity::Entity(int32_t id, const glm::vec3 &position): id(id) {
    transform.position = position;
    smooth_transform.position = position;
    init();
}

Entity::Entity(int32_t id, const glm::vec3 &position, const std::string& name): id(id), name(name) {
    transform.position = position;
    smooth_transform.position = position;
    init();
}

void Entity::init() {
    const GLfloat vertices[] = {
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

    const GLuint indices[] = {
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

    indices_count = sizeof(indices) / sizeof(GLuint);

    VBO = createBufferStorage(vertices, sizeof(vertices), GL_DYNAMIC_STORAGE_BIT);
    EBO = createBufferStorage(indices, sizeof(indices), GL_DYNAMIC_STORAGE_BIT);
    VAO = createVAO(VBO, "3f", EBO);
}

void Entity::draw() const
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, 0);
}
