#pragma once

#include <vector>
#include <string>

#include "glm/ext/vector_int3.hpp"
#include "enums.hpp"

struct Blueprint {
    glm::ivec3 dimensions;

    // positions should be between 0,0,0 and dimensions
    std::vector<std::tuple<glm::ivec3, BlockType>> blocks;
};

Blueprint createBlueprintFromSelection(const glm::ivec3 min, const glm::ivec3 max);
void deleteBlueprint(const char* name);
void saveBlueprintToFile(const Blueprint& blueprint, const std::string& name);
Blueprint createBlueprintFromFile(const std::string& name);
void pasteBlueprintIntoWorld(const Blueprint& bp, const glm::ivec3& pos);
