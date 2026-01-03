#include "GameView.hpp"
#include "Client.hpp"
#include "World.hpp"
#include "world_to_screen_space.hpp"
#include "clock.hpp"
#include "string_helpers.hpp"
#include "mem_info.h"
#include <imgui.h>

void GameView::drawPlayersNames()
{
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoNav;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoInputs;
    // window_flags |= ImGuiWindowFlags_NoBackground;

    int32_t idx = 0;
    for (const Entity& e : World::instance().entities) {

        glm::ivec2 screen_pos = worldToScreenSpace(e.smooth_transform.position + glm::vec3(0.0f, 0.8f, 0.0f), camera.getProjection(), camera.getView(), ctx.width, ctx.height);
        screen_pos.y -= 20;
        if (glm::dot(camera.forward(), glm::normalize(e.smooth_transform.position - camera.getPosition())) < 0.2f) {
            continue;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::SetNextWindowSizeConstraints({0, 0}, {FLT_MAX, FLT_MAX});
        ImGui::SetNextWindowBgAlpha(0.2f);
        ImGui::SetNextWindowSize({0, 0});
        ImGui::SetNextWindowPos({(float)screen_pos.x, (float)screen_pos.y}, 0, {0.5f, 0.5f});
        ImGui::Begin(("##Name" + std::to_string(idx)).c_str(), nullptr, window_flags);
        ImGui::Text("%s", e.name.c_str());
        ImGui::End();
        ImGui::PopStyleVar();

        idx += 1;
    }
}

void GameView::gui(float dt)
{
    // ImGui::ShowDemoWindow();

    if (block_selection_mode) {
        guiWorldEdit();
    }

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Debug", nullptr, !_cursor_enabled ? ImGuiWindowFlags_NoInputs : 0);

    if (ImGui::Checkbox("VSync", &_vsync)) {
        ctx.setVsync(_vsync);
    }

    ImGui::Checkbox("Profiler", &_show_profiler_gui);

    ImGui::Text("%.4f secs", dt);
    ImGui::Text("%.2f fps", 1.0f / dt);

    // ImGui::Text("%s", SimpleProfiler::instance().dump().c_str());
    ImGui::Text("RAM: %.4f / %.4f Go", ((double)getCurrentRSS()) / (1024*1024*1024), ((double)getPeakRSS()) / (1024*1024*1024));
    ImGui::Text("BufferVertices: %d / %d - %d", world_renderer.buffer_allocator_vertices.getAvailableMemory(), world_renderer.buffer_allocator_vertices.getMaxMemory(), world_renderer.buffer_allocator_vertices.getSlotCount());

    ImGui::Text("ThreadPool{%d} tasks: %d", (int32_t)world_renderer.thread_pool._workers.size(), (int32_t)world_renderer.thread_pool._task_queue.size());
    ImGui::Text("New chunks: %d", (int32_t)Client::instance().new_chunks.size());
    ImGui::Text("Chunks: %d (%d rendered)", World::instance().getChunkCount(), world_renderer.chunks_drawn);

    ImGui::SliderFloat("FOV", &camera.fov, 1.0f, 179.0f, "%.0f");
    glm::vec3 camera_pos = camera.getPosition();
    ImGui::Text("position: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
    ImGui::Text("forward: %.2f, %.2f, %.2f", camera.forward().x, camera.forward().y, camera.forward().z);
    ImGui::Text("block in hand: %d", (int32_t)block_in_hand);

    ImGui::Checkbox("FreeCam", &free_cam);
    ImGui::Checkbox("World edit", &block_selection_mode);
    ImGui::SliderFloat("Bulk Edit Radius: ", &bulk_edit_radius, 1.0f, 32.0f, "%.2f");

    ImGui::Text("ClientId: %d", Client::instance().client_id);

    if (ImGui::TreeNode(SC("Entities: " << World::instance().entities.size()))) {
        for (auto& entity : World::instance().entities) {
            ImGui::PushID(entity.id);
            ImGui::Text("id:%d: x:%.2f y:%.2f z:%.2f", entity.id, entity.transform.position.x, entity.transform.position.y, entity.transform.position.z);
            ImGui::Text("name: %s", entity.name.c_str());
            ImGui::SameLine();
            if (ImGui::Button("teleport")) {
                setPlayerPosition(entity.transform.position);
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    ImGui::Checkbox("Chunks borders", &_draw_chunks_borders);
    ImGui::Checkbox("Player Chunk borders", &_draw_player_chunk);
    ImGui::Checkbox("Draw player colliders", &_draw_player_colliders);
    ImGui::Checkbox("Draw cursor hitpoint", &_draw_hit_point);
    ImGui::Checkbox("Delete far chunks", &_delete_far_chunks);

    ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 260), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& msg: tchat) {
            ImGui::TextWrapped("%s", msg.c_str());
            ImGui::Spacing();
        }
    ImGui::EndChild();

    ImGui::InputText("##inputText", input_text_buffer, IM_ARRAYSIZE(input_text_buffer));
    ImGui::SameLine();
    if (ImGui::Button("Send")) {
        sendTextMessage();
    }

    ImGui::NewLine();

    world_renderer.imguiRender();

    ImGui::End();
}
