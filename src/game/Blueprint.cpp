#include "Blueprint.hpp"

#include <iostream>
#include <fstream>
#include <regex>
#include <cstdio>

#include "enums.hpp"
#include "World.hpp"
#include "Client.hpp"

static const std::string BLUEPRINT_FOLDER_PATH = "./blueprints/";

Blueprint createBlueprintFromSelection(const glm::ivec3 min, const glm::ivec3 max) {
    std::vector<std::tuple<glm::ivec3, BlockType>> blocks;

    for (int z = min.z ; z <= max.z ; ++z) {
    for (int y = min.y ; y <= max.y ; ++y) {
    for (int x = min.x ; x <= max.x ; ++x) {
        const BlockType block = World::instance().getBlock(glm::ivec3(x, y, z));
        if (block == BlockType::Air) continue;
        blocks.emplace_back(
            glm::ivec3{x, y, z} - min,
            block
        );
    }}}

    return { (max+1) - min, blocks };
}

void deleteBlueprint(const char* name) {
    int r = std::remove((BLUEPRINT_FOLDER_PATH + name).c_str());
    if (r != 0) {
        printf("Error: Couldn't remove file \"%s\"\n", name);
    }
}

void saveBlueprintToFile(const Blueprint& blueprint, const std::string& name)
{
    std::ofstream file;
    file.open(BLUEPRINT_FOLDER_PATH + name);

    file << blueprint.dimensions.x << ',' << blueprint.dimensions.y << ',' << blueprint.dimensions.z << '\n';
    for (const auto& [pos, blocktype] : blueprint.blocks) {
        file << pos.x << ',' << pos.y << ',' << pos.z << ',' << static_cast<int>(blocktype) << '\n';
    }

    file.close();
}

Blueprint createBlueprintFromFile(const std::string& name) {
    Blueprint bp = {};
    std::ifstream file;
    std::string str = "";
    std::smatch m = {};
    file.open(name);

    std::getline(file, str);

    std::regex regex_dimensions("(-?\\d+),(-?\\d+),(-?\\d+)");
    std::regex regex_block("(-?\\d+),(-?\\d+),(-?\\d+),(\\d+)");

    if (!std::regex_match(str, m, regex_dimensions)) {
        // ERROR
    }


    bp.dimensions = glm::ivec3(std::stoi(m[1]), std::stoi(m[2]), std::stoi(m[3]));

    while (std::getline(file, str)) {
        if (std::regex_match(str, m, regex_block)) {
            bp.blocks.push_back(
                std::tuple(glm::ivec3{std::stoi(m[1]), std::stoi(m[2]), std::stoi(m[3])}, (BlockType)std::stoi(m[4]))
            );
        } else {
            break;
        }
    }

    file.close();
    return bp;
}

void pasteBlueprintIntoWorld(const Blueprint& bp, const glm::ivec3& pos)
{
    // Send in chunks of 2048 blocks

    int i = 0;
    while (1) {
        int next_i = glm::clamp(i + 2048, 0, (int)bp.blocks.size());
        if (i > next_i) break;

        const auto& first = bp.blocks.begin() + i;
        const auto& last = bp.blocks.begin() + next_i;
        std::vector<std::tuple<glm::ivec3, BlockType>> part_blocks(first, last);

        for (auto& [local_pos, blocktype] : part_blocks) {
            local_pos += pos;
        }

        if (part_blocks.size() > 0) {
            Client::instance().sendBlockBulkEditPacket(part_blocks);
        } else {
            break;
        }

        i += 2048;
    }
}


// void pasteBlueprintIntoWorld(const Blueprint& bp, const glm::ivec3& pos)
// {
//     std::vector<std::tuple<glm::ivec3, BlockType>> blocks(bp.blocks);

//     for (auto& [local_pos, blocktype] : blocks) {
//         local_pos += pos;
//     }

//     Client::instance().sendBlockBulkEditPacket(blocks);
// }
