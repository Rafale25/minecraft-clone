#include "GameState.hpp"

static _GameState global_gamestate;

namespace GameState {
    void setRenderDistance(int value) {
        value = value > MAX_RENDER_DISTANCE ? MAX_RENDER_DISTANCE: value;
        value = value < MIN_RENDER_DISTANCE ? MIN_RENDER_DISTANCE: value;
        global_gamestate.render_distance = value;
    }

    int32_t getRenderDistance() {
        return global_gamestate.render_distance;
    }
}
