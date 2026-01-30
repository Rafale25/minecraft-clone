#include "WorldRenderer.hpp"
#include <imgui.h>

double map(double value, double min1, double max1, double min2, double max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

static void ImguiCheckboxInt(const char* title, int& value) {
    bool checked = value != 0;
    if (ImGui::Checkbox(title, &checked))
        value = checked ? 1 : 0;
}

void WorldRenderer::imguiRender()
{
    if (!ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::Checkbox("Wireframe", &m_wireframeEnabled);

    static bool _shadowmap_texture = false;
    ImGui::Checkbox("ShadowMap texture", &_shadowmap_texture);
    if (_shadowmap_texture) {
        ImGui::Begin("Shadow map");
        // ImGui::Image((ImTextureID)(intptr_t) world_renderer.shadowmap._depthTexture._texture, ImVec2(world_renderer.shadowmap._shadowmap_size/4, world_renderer.shadowmap._shadowmap_size/4), ImVec2(0, 1), ImVec2(1, 0));
        for (int i = 0 ; i < 4 ; ++i) {
            ImGui::Image((ImTextureID)(intptr_t) m_textureView[i], ImVec2(m_shadowmap.getShadowmapSize()/8, m_shadowmap.getShadowmapSize()/8), ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::End();
    }

    static bool _gbuffer_window = false;
    ImGui::Checkbox("G-Buffer window", &_gbuffer_window);
    if (_gbuffer_window) {
        ImGui::Begin("G-buffer");
            ImGui::BeginGroup();
                ImGui::Text("World Position");
                ImGui::Image((ImTextureID)(intptr_t) m_textureWorldPosition._texture, ImVec2(m_textureWorldPosition._width/4, m_textureWorldPosition._height/4), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
                ImGui::Text("Normals");
                ImGui::Image((ImTextureID)(intptr_t) m_textureNormals._texture, ImVec2(m_textureNormals._width/4, m_textureNormals._height/4), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::EndGroup();
            ImGui::BeginGroup();
                ImGui::Text("Volumetrics");
                ImGui::Image((ImTextureID)(intptr_t) m_textureVolumetrics._texture, ImVec2(m_textureVolumetrics._width/1, m_textureVolumetrics._height/1), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::EndGroup();
        ImGui::End();
    }

    ImGui::SliderAngle("Sun Rotation", &m_sunRotation, 0.0f, 360.0f);
    ImGui::SliderAngle("Sun Pitch", &m_sunPitch, 0.0f, 90.0f);
    ImGui::SliderAngle("Sun Yaw", &m_sunYaw, 0.0f, 360.0f);
    ImGui::SliderFloat("Shadow Bias", &m_shadowmap.m_shadowBias, 0.000001f, 0.001f, "%.6f");
    ImGui::SliderFloat("Shadow Distance", &m_maxShadowDistance, 0.3f, 2000.0f, "%.2f");
    ImGui::SliderFloat("Fog density", &m_uniformParameters.fogDensity, 0.0f, 0.05f, "%.6f", ImGuiSliderFlags_Logarithmic);
    ImguiCheckboxInt("Ambiant occlusion", m_uniformParameters.ambient_occlusion_enabled);
    ImGui::SliderFloat("AO strength", &m_uniformParameters.ambient_occlusion_strength, 0.0f, 1.0f, "%.2f");
    ImguiCheckboxInt("Tonemapping", m_uniformParameters.tonemapping_enabled);
    ImGui::SliderFloat("Exposure", &m_uniformParameters.exposure, 0.0f, 10.0f, "%.3f");
    ImGui::Checkbox("Freeze shadowmap camera", &m_isShadowCameraFreezed);
    ImGui::Checkbox("Debug draw shadowmap frustums", &m_debugDrawShadowmapFrustums);
    ImguiCheckboxInt("Shadows Enabled", m_uniformParameters.shadows_enabled);
    ImGui::SliderFloat("Shadow Cascade 1", &m_shadowmap.shadowCascadeLevels[0], 1.0f, 1000.0f, "%.1f");
    ImGui::SliderFloat("Shadow Cascade 2", &m_shadowmap.shadowCascadeLevels[1], 1.0f, 1000.0f, "%.1f");
    ImGui::SliderFloat("Shadow Cascade 3", &m_shadowmap.shadowCascadeLevels[2], 1.0f, 1000.0f, "%.1f");
    ImGui::SliderFloat("Shadow Cascade 4", &m_shadowmap.shadowCascadeLevels[3], 1.0f, 10'000.0f, "%.1f");
    ImGui::SliderFloat("Volumetric Density", &m_uniformParameters.volumetricDensity, 0.0f, 0.05f, "%.5f");
    ImGui::SliderFloat("Volumetric HGphase Front", &m_uniformParameters.volumetricHGphaseFront, 0.0f, 1.0f, "%.4f");
    ImGui::SliderFloat("Volumetric HGphase Back", &m_uniformParameters.volumetricHGphaseBack, -1.0f, 0.0f, "%.4f");
    ImGui::SliderFloat("Volumetric Ambient light", &m_uniformParameters.volumetricAmbiantLight, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("TEST_SLIDER_0", &m_uniformParameters.TEST_SLIDER_0, -1.0f, 1.0f, "%.6f");
    ImGui::SliderFloat("TEST_SLIDER_1", &m_uniformParameters.TEST_SLIDER_1, -1.0f, 1.0f, "%.6f");


    static bool _memoryAllocatorWindow = false;
    ImGui::Checkbox("Memory-Allocator Window", &_memoryAllocatorWindow);
    if (_memoryAllocatorWindow) {
        ImGui::Begin("Vertex buffer");
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 widgetPos = ImGui::GetCursorScreenPos();
            ImVec2 regionSize = ImGui::GetContentRegionAvail();

            float width = regionSize.x;
            float height = regionSize.y;//80.0f;

            const float maxMemory = m_bufferAllocatorVertices.m_maxMemory;

            for (const auto& slot : m_bufferAllocatorVertices.m_slots) {
                if (!slot.used) continue;

                double left = map(slot.start, 0.0, maxMemory, 0.0, width);
                double right = map(slot.start + slot.size, 0.0, maxMemory, 0.0, width);

                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(static_cast<float>(slot.start % 360) / 360.0f, 1.0f, 1.0f, r, g, b);

                drawList->AddRectFilled(
                    ImVec2(widgetPos.x + left, widgetPos.y),
                    ImVec2(widgetPos.x + right, widgetPos.y + height),
                    ImGui::GetColorU32(ImVec4(r, g, b, 1.0f))
                    // ImGui::GetColorU32()
                );
            }
        ImGui::End();
    }
}
