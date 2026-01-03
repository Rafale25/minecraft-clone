#pragma once

#include <glm/ext/vector_int3.hpp>
class Camera;

class IWorldRenderer {
// public:
    virtual void onDeletedChunk(const glm::ivec3& chunk_pos) = 0;
    virtual void onAddedChunk(const glm::ivec3& chunk_pos) = 0;
    virtual void onResize(int32_t width, int32_t height) = 0;


    virtual void render(const Camera &camera) = 0;
    virtual void update() = 0;

    virtual void imguiRender() = 0;
};
