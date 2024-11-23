#include "WorldRenderer.hpp"

#include "Frustum.hpp"
#include "World.hpp"
#include "Chunk.hpp"
#include "Camera.hpp"
#include "BlockTextureManager.hpp"

#include <shared_mutex>

WorldRenderer::WorldRenderer(Context &context): _ctx(context)
{
    chunk_vao = createVAO(0, "i");

    draw_command_buffer = createBufferStorage(nullptr, sizeof(DrawElementsIndirectCommand) * MAX_COMMANDS, GL_DYNAMIC_STORAGE_BIT);
    ssbo_chunk_positions = createBufferStorage(nullptr, sizeof(GLfloat)*4 * MAX_COMMANDS, GL_DYNAMIC_STORAGE_BIT);

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
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_texture_handles);

    Frustum camera_frustum = createFrustumFromCamera(camera, camera.aspect_ratio, glm::radians(camera.fov), camera.near_plane, camera.far_plane);

    chunks_drawn = 0;

    std::vector<DrawElementsIndirectCommand> commands;
    std::vector<glm::vec4> chunk_positions;

    const std::shared_lock<std::shared_mutex> lock(World::instance().chunks_mutex);

    program.setVec3("u_chunkPos", {0.0f, 0.0f, 0.0f});

    for (const auto& [key, chunk] : World::instance().chunks)
    {
        if (chunk->mesh.slot_vertices.id == -1) continue;
        if (chunk->mesh.slot_indices.id == -1) continue;

        if (use_frustum_culling) {
            AABB chunk_aabb = {(chunk->pos * 16), (chunk->pos * 16) + 16};
            if (!chunk_aabb.isOnFrustum(camera_frustum)) continue;
        }

        chunk_positions.push_back(glm::vec4(chunk->pos * 16, 1.0f));

        commands.emplace_back(
            chunk->mesh.slot_indices.size / sizeof(GLuint),
            1,
            chunk->mesh.slot_indices.start / sizeof(GLuint),
            chunk->mesh.slot_vertices.start / sizeof(GLuint),
            0
        );

        ++chunks_drawn;
    }
    glBindVertexArray(chunk_vao);
    glVertexArrayVertexBuffer(chunk_vao, 0, buffer_allocator_vertices.getBufferObject(), 0, 1 * sizeof(GLuint));
    glVertexArrayElementBuffer(chunk_vao, buffer_allocator_indices.getBufferObject());

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_command_buffer);
    glNamedBufferSubData(draw_command_buffer, 0, sizeof(DrawElementsIndirectCommand) * commands.size(), (const void *)commands.data());

    glNamedBufferSubData(ssbo_chunk_positions, 0, sizeof(GLfloat) * 4 * chunk_positions.size(), (const void *)chunk_positions.data());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_chunk_positions);

    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_texture_handles);

    // printf("commands %d\n", commands.size());
    // for (DrawElementsIndirectCommand &cmd : commands) {
    //     printf("Cmd: %d %d %d %d %d\n", cmd.count, cmd.instanceCount, cmd.firstIndex, cmd.baseVertex, cmd.baseInstance);
    // }

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void *)0, commands.size(), 0);
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
