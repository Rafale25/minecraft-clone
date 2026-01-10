#pragma once

#include <cstdint>

constexpr int32_t MIN_RENDER_DISTANCE = 1;
constexpr int32_t MAX_RENDER_DISTANCE = 64;

struct _GameState {
    int32_t renderDistance = MIN_RENDER_DISTANCE;
};

namespace GameState {
    void setRenderDistance(int value);
    int32_t getRenderDistance();
}
