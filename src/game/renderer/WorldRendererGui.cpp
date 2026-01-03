#include "WorldRenderer.hpp"
#include <imgui.h>

static void ImguiCheckboxInt(const char* title, int& value) {
    bool checked = value != 0;
    if (ImGui::Checkbox(title, &checked))
        value = checked ? 1 : 0;
}

void WorldRenderer::imguiRender()
{
    if (!ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::Checkbox("Wireframe", &_wireframe);

    static bool _shadowmap_texture = false;
    ImGui::Checkbox("ShadowMap texture", &_shadowmap_texture);
    if (_shadowmap_texture) {
        ImGui::Begin("Shadow map");
        // ImGui::Image((ImTextureID)(intptr_t) world_renderer.shadowmap._depthTexture._texture, ImVec2(world_renderer.shadowmap._shadowmap_size/4, world_renderer.shadowmap._shadowmap_size/4), ImVec2(0, 1), ImVec2(1, 0));
        for (int i = 0 ; i < 4 ; ++i) {
            ImGui::Image((ImTextureID)(intptr_t) _texture_view[i], ImVec2(shadowmap.getShadowmapSize()/8, shadowmap.getShadowmapSize()/8), ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::End();
    }

    static bool _gbuffer_window = false;
    ImGui::Checkbox("G-Buffer window", &_gbuffer_window);
    if (_gbuffer_window) {
        ImGui::Begin("G-buffer");
            ImGui::BeginGroup();
                ImGui::Text("World Position");
                ImGui::Image((ImTextureID)(intptr_t) _texture_world_position._texture, ImVec2(_texture_world_position._width/4, _texture_world_position._height/4), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
                ImGui::Text("Normals");
                ImGui::Image((ImTextureID)(intptr_t) _texture_normals._texture, ImVec2(_texture_normals._width/4, _texture_normals._height/4), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::EndGroup();
            ImGui::BeginGroup();
                ImGui::Text("Volumetrics");
                ImGui::Image((ImTextureID)(intptr_t) _texture_volumetrics._texture, ImVec2(_texture_volumetrics._width/1, _texture_volumetrics._height/1), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::EndGroup();
        ImGui::End();
    }

    ImGui::SliderAngle("Sun Rotation", &_sun_rotation, 0.0f, 360.0f);
    ImGui::SliderAngle("Sun Pitch", &_sun_pitch, 0.0f, 90.0f);
    ImGui::SliderAngle("Sun Yaw", &_sun_yaw, 0.0f, 360.0f);
    ImGui::SliderFloat("Shadow Bias", &shadowmap._shadow_bias, 0.000001f, 0.001f, "%.6f");
    ImGui::SliderFloat("Shadow Distance", &_max_shadow_distance, 0.3f, 2000.0f, "%.2f");
    ImGui::SliderFloat("Fog density", &uniform_parameters.fogDensity, 0.0f, 0.05f, "%.6f", ImGuiSliderFlags_Logarithmic);
    ImguiCheckboxInt("Ambiant occlusion", uniform_parameters.ambient_occlusion_enabled);
    ImGui::SliderFloat("AO strength", &uniform_parameters.ambient_occlusion_strength, 0.0f, 1.0f, "%.2f");
    ImguiCheckboxInt("Tonemapping", uniform_parameters.tonemapping_enabled);
    ImGui::SliderFloat("Exposure", &uniform_parameters.exposure, 0.0f, 10.0f, "%.3f");
    ImGui::Checkbox("Freeze shadowmap camera", &_is_shadow_camera_freezed);
    ImGui::Checkbox("Debug draw shadowmap frustums", &_debug_draw_shadowmap_frustums);
    ImguiCheckboxInt("Shadows Enabled", uniform_parameters.shadows_enabled);
    ImGui::SliderFloat("Shadow Cascade 1", &shadowmap.shadowCascadeLevels[0], 1.0f, 1000.0f, "%.1f");
    ImGui::SliderFloat("Shadow Cascade 2", &shadowmap.shadowCascadeLevels[1], 1.0f, 1000.0f, "%.1f");
    ImGui::SliderFloat("Shadow Cascade 3", &shadowmap.shadowCascadeLevels[2], 1.0f, 1000.0f, "%.1f");
    ImGui::SliderFloat("Shadow Cascade 4", &shadowmap.shadowCascadeLevels[3], 1.0f, 10'000.0f, "%.1f");
    ImGui::SliderFloat("Volumetric Density", &uniform_parameters.volumetricDensity, 0.0f, 0.05f, "%.5f");
    ImGui::SliderFloat("Volumetric HGphase Front", &uniform_parameters.volumetricHGphaseFront, 0.0f, 1.0f, "%.4f");
    ImGui::SliderFloat("Volumetric HGphase Back", &uniform_parameters.volumetricHGphaseBack, -1.0f, 0.0f, "%.4f");
    ImGui::SliderFloat("Volumetric Ambient light", &uniform_parameters.volumetricAmbiantLight, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("TEST_SLIDER_0", &uniform_parameters.TEST_SLIDER_0, -1.0f, 1.0f, "%.6f");
    ImGui::SliderFloat("TEST_SLIDER_1", &uniform_parameters.TEST_SLIDER_1, -1.0f, 1.0f, "%.6f");

}
