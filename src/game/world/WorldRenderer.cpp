#include "WorldRenderer.hpp"

#include "Frustum.hpp"
#include "World.hpp"
#include "Chunk.hpp"
#include "Camera.hpp"
#include "BlockTextureManager.hpp"

#include <shared_mutex>

WorldRenderer::WorldRenderer(Context &context): _ctx(context)
{
    cube_shader.use();
    cube_shader.setInt("shadowMap", 0);

    BlockTextureManager::loadAllTextures();
    ssbo_texture_handles = createBufferStorage(&BlockTextureManager::Get().textures_handles[0], BlockTextureManager::Get().textures_handles.size() * sizeof(GLuint64));
}

void WorldRenderer::setDefaultRenderState()
{
    glEnable(GL_MULTISAMPLE); // enabled by default

    glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    // glEnable(GL_FRAMEBUFFER_SRGB);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void WorldRenderer::render(const Camera &camera)
{
    setDefaultRenderState();

    renderShadowmap(camera);

    renderSkybox(camera);

    cube_shader.use();
    cube_shader.setMat4("u_lightSpaceMatrix", shadowmap._lightSpaceMatrix);
    cube_shader.setVec3("u_sun_direction", sunDir);
    cube_shader.setFloat("u_shadow_bias", shadowmap._shadow_bias);
    cube_shader.setFloat("u_ambient_occlusion_enabled", _ambient_occlusion);
    cube_shader.setFloat("u_ambient_occlusion_strength", _ambient_occlusion_strength);
    cube_shader.setBool("u_AO_squared", _AO_squared);

    glBindTextureUnit(0, shadowmap._depthTexture._texture);
    renderTerrain(cube_shader, camera);
    renderEntities(camera);
}

void WorldRenderer::renderTerrain(const Program& program, const Camera &camera, bool use_frustum_culling)
{
    program.use();
    program.setMat4("u_projectionMatrix", camera.getProjection());
    program.setMat4("u_viewMatrix", camera.getView());
    program.setVec3("u_view_position", camera.getPosition());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_texture_handles);

    Frustum camera_frustum = createFrustumFromCamera(camera, camera.aspect_ratio, glm::radians(camera.fov), camera.near_plane, camera.far_plane);

    chunks_drawn = 0;

    GLuint vao = World::instance().chunk_vao;
    glBindVertexArray(vao);

    const std::shared_lock<std::shared_mutex> lock(World::instance().chunks_mutex);

    for (const auto& [key, chunk] : World::instance().chunks)
    {
        if (chunk->mesh.indices_count == 0 || chunk->mesh.VBO == 0) continue;

        if (use_frustum_culling) {
            AABB chunk_aabb = {(chunk->pos * 16), (chunk->pos * 16) + 16};
            if (!chunk_aabb.isOnFrustum(camera_frustum)) continue;
        }

        program.setVec3("u_chunkPos", chunk->pos * 16);

        glVertexArrayVertexBuffer(vao, 0, chunk->mesh.VBO, 0, 1 * sizeof(GLuint));
        glVertexArrayElementBuffer(vao, chunk->mesh.EBO);
        glDrawElements(GL_TRIANGLES, chunk->mesh.indices_count, GL_UNSIGNED_INT, 0);
        ++chunks_drawn;
    }
}

void WorldRenderer::renderTerrainDepth(const Camera &camera)
{
    renderTerrain(cube_shadowmapping_shader, camera, false);
}

void WorldRenderer::renderEntities(const Camera &camera)
{
    mesh_shader.use();
    mesh_shader.setMat4("u_projectionMatrix", camera.getProjection());
    mesh_shader.setMat4("u_viewMatrix", camera.getView());
    for (auto& entity : World::instance().entities)
    {
        mesh_shader.setMat4("u_modelMatrix", entity.smooth_transform.getMatrix());
        entity.draw();
    }
}

void WorldRenderer::renderEntitiesDepth(const Camera &camera)
{
}

void WorldRenderer::renderSkybox(const Camera &camera)
{
    glDisable(GL_DEPTH_TEST);
    skybox_shader.use();
    glm::mat4 view_rotation = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), -camera.forward(), glm::vec3(0.0f, 1.0f, 0.0f)); // wtf
    skybox_shader.setVec2("u_resolution", glm::vec2(_ctx.width, _ctx.height));
    skybox_shader.setMat4("u_view", view_rotation);
    skybox_shader.setFloat("u_sunDotAngle", glm::dot(sunDir, {0.0f, 1.0f, 0.0f}));
    skybox_quad.draw();
    glEnable(GL_DEPTH_TEST);
}

void WorldRenderer::renderShadowmap(const Camera &camera)
{
    shadowmap.setSunDir(sunDir);
    shadowmap.begin(camera, cube_shadowmapping_shader);
        renderTerrain(cube_shadowmapping_shader, camera, false);
    shadowmap.end();
}
