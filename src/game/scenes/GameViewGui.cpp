#include "GameView.hpp"
#include "Client.hpp"
#include "World.hpp"
#include "worldToScreenSpace.hpp"
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
    for (const Entity& e : World::instance().m_entities) {

        glm::ivec2 screen_pos = worldToScreenSpace(e.smoothTransform.position + glm::vec3(0.0f, 0.8f, 0.0f), m_camera.getProjection(), m_camera.getView(), ctx.width, ctx.height);
        screen_pos.y -= 20;
        if (glm::dot(m_camera.forward(), glm::normalize(e.smoothTransform.position - m_camera.getPosition())) < 0.2f) {
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

    if (m_blockSelectionMode) {
        guiWorldEdit();
    }

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Debug", nullptr, !m_cursorEnabled ? ImGuiWindowFlags_NoInputs : 0);

    if (ImGui::Button("Refresh Scripts")) {
        m_scriptManager.refresh();
    }

    if (ImGui::Checkbox("VSync", &m_vsyncEnabled)) {
        ctx.setVsync(m_vsyncEnabled);
    }

    ImGui::Checkbox("Profiler", &m_showProfilerGui);

    ImGui::Text("%.4f secs", dt);
    ImGui::Text("%.2f fps", 1.0f / dt);

    // ImGui::Text("%s", SimpleProfiler::instance().dump().c_str());
    ImGui::Text("RAM: %.4f / %.4f Go", ((double)getCurrentRSS()) / (1024*1024*1024), ((double)getPeakRSS()) / (1024*1024*1024));
    ImGui::Text("BufferVertices: %d / %d - %d", m_worldRenderer.m_bufferAllocatorVertices.getAvailableMemory(), m_worldRenderer.m_bufferAllocatorVertices.getMaxMemory(), m_worldRenderer.m_bufferAllocatorVertices.getSlotCount());

    ImGui::Text("ThreadPool{%d} tasks: %d", (int32_t)m_worldRenderer.m_threadPool.m_workers.size(), (int32_t)m_worldRenderer.m_threadPool.m_taskQueue.size());
    ImGui::Text("New chunks: %d", (int32_t)Client::instance().m_newChunks.size());
    ImGui::Text("Chunks: %d (%d rendered)", World::instance().getChunkCount(), m_worldRenderer.m_chunksDrawn);

    ImGui::SliderFloat("FOV", &m_camera.fov, 1.0f, 179.0f, "%.0f");
    glm::vec3 camera_pos = m_camera.getPosition();
    ImGui::Text("position: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
    ImGui::Text("forward: %.2f, %.2f, %.2f", m_camera.forward().x, m_camera.forward().y, m_camera.forward().z);
    ImGui::Text("block in hand: %d", (int32_t)m_blockInHand);

    ImGui::Checkbox("FreeCam", &m_freeCamEnabled);
    ImGui::Checkbox("World edit", &m_blockSelectionMode);
    ImGui::SliderFloat("Bulk Edit Radius: ", &m_bulkEditRadius, 1.0f, 32.0f, "%.2f");

    ImGui::Text("ClientId: %d", Client::instance().m_clientId);

    if (ImGui::TreeNode(std::format("Entities: {}", World::instance().m_entities.size()).c_str())) {
        for (auto& entity : World::instance().m_entities) {
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

    ImGui::Checkbox("Chunks borders", &m_drawChunksBorders);
    ImGui::Checkbox("Player Chunk borders", &m_drawPlayerChunk);
    ImGui::Checkbox("Draw player colliders", &m_drawPlayerColliders);
    ImGui::Checkbox("Draw cursor hitpoint", &m_drawHitPoint);
    ImGui::Checkbox("Delete far chunks", &m_deleteFarChunks);

    ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 260), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& msg: m_tchat) {
            ImGui::TextWrapped("%s", msg.c_str());
            ImGui::Spacing();
        }
    ImGui::EndChild();

    ImGui::InputText("##inputText", m_inputTextBuffer, IM_ARRAYSIZE(m_inputTextBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Send")) {
        sendTextMessage();
    }

    ImGui::NewLine();

    ImGui::NewLine();

    m_worldRenderer.imguiRender();

    ImGui::Dummy({0.0f, ImGui::GetWindowSize().y});

    ImGui::End();
}
