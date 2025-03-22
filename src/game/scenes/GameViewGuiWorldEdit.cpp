#include <filesystem>
#include <algorithm>

#include "GameView.hpp"
#include "Blueprint.hpp"
#include "imgui.h"

struct BlueprintFileInfo {
    std::filesystem::directory_entry entry;
    std::string name;
    int32_t file_size;
};

void GameView::guiWorldEdit()
{
    static Blueprint bp = {};
    static int32_t selected_index = -1;

    ImGui::Begin("WorldEdit", nullptr, !_cursor_enabled ? ImGuiWindowFlags_NoInputs : 0);

    const std::string path = "blueprints";
    std::vector<BlueprintFileInfo> blueprint_infos;

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        blueprint_infos.push_back({
            entry,
            entry.path().filename().replace_extension().string(),
            (int32_t)(entry.file_size())
        });
    }

    if (ImGui::Button("Save Selection as Blueprint")) {
        const glm::ivec3 min = glm::min(blockA, blockB);
        const glm::ivec3 max = glm::max(blockA, blockB);
        bp = createBlueprintFromSelection(min, max);

        ImGui::OpenPopup("SaveBlueprint");
    }

    if (ImGui::BeginPopupModal("SaveBlueprint", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static bool name_already_taken = false;
        static char input_text[64] = {};
        ImGui::InputText("name", input_text, IM_ARRAYSIZE(input_text));

        if (name_already_taken) {
            ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "Name already taken");
            ImGui::Separator();
        }

        if (!name_already_taken && ImGui::Button("Confirm", ImVec2(120, 0))) {
            const auto& it = std::find_if(blueprint_infos.begin(), blueprint_infos.end(), [&](const BlueprintFileInfo& bp_info) { return bp_info.name == input_text; });
            name_already_taken = it != blueprint_infos.end();

            if (!name_already_taken) {
                saveBlueprintToFile(bp, input_text);
                memset(input_text, 0, sizeof(input_text));
                name_already_taken = false;
                ImGui::CloseCurrentPopup();
            }
        }

        if (name_already_taken && ImGui::Button("Overwrite", ImVec2(120, 0))) {
            saveBlueprintToFile(bp, input_text);
            memset(input_text, 0, sizeof(input_text));
            name_already_taken = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("DeleteBlueprint", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "Delete blueprint %s", blueprint_infos[selected_index].name.c_str());
        ImGui::Separator();

        if (ImGui::Button("Confirm", ImVec2(120, 0))) {
            deleteBlueprint(blueprint_infos[selected_index].name.c_str());
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (selected_index != -1) {
        if (ImGui::Button("Load Blueprint from file and paste")) {
            if (selected_index == -1) {
                ImGui::OpenPopup("##NoBlueprintSelected");
            } else {
                const Blueprint& bp = createBlueprintFromFile(blueprint_infos[selected_index].entry.path().string());
                // printf("%d %d %d\n", bp.dimensions.x, bp.dimensions.y, bp.dimensions.z);
                // printf("%d\n", (int32_t)bp.blocks.size());
                pasteBlueprintIntoWorld(bp, player_blockraycasthit.block_pos);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete")) {
                ImGui::OpenPopup("DeleteBlueprint");
        }
    }

    ImGui::Separator();

    ImGui::Text("Blueprints");
    if (ImGui::BeginListBox(""))
    {
        for (int32_t i = 0 ; i < (int32_t)blueprint_infos.size() ; ++i) {
            if (ImGui::Selectable(blueprint_infos[i].name.c_str(), i == selected_index)) {
                selected_index = i;
            }
            ImGui::SameLine();
            ImGui::Text("%.2f Ko", (float)blueprint_infos[i].file_size * 0.001f);
        }

        ImGui::EndListBox();
    }

    ImGui::End();
}
