#include "WorldRenderer.hpp"

#include "Frustum.hpp"
#include "World.hpp"
#include "Chunk.hpp"
#include "Camera.hpp"
#include "BlockTextureManager.hpp"

WorldRenderer::WorldRenderer(Context &context): _ctx(context)
{
    chunk_vao = createVAO(0, "i");

    draw_command_buffer = createBufferStorage(nullptr, sizeof(DrawElementsIndirectCommand) * MAX_COMMANDS, GL_DYNAMIC_STORAGE_BIT);
    ssbo_chunk_positions = createBufferStorage(nullptr, sizeof(GLfloat)*4 * MAX_COMMANDS, GL_DYNAMIC_STORAGE_BIT);

    cube_shader.use();
    cube_shader.setInt("shadowMap", 0);

    BlockTextureManager::loadAllTextures();
    ssbo_texture_handles = createBufferStorage(BlockTextureManager::Get().textures_handles.data(), BlockTextureManager::Get().textures_handles.size() * sizeof(GLuint64));

    onResize(context.width, context.height);
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
}

void WorldRenderer::render(const Camera &camera)
{
    setDefaultRenderState();

    renderShadowmap(camera);

    _framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // renderSkybox(camera);

    cube_shader.use();
    cube_shader.setMat4("u_lightSpaceMatrix", shadowmap._lightSpaceMatrix);
    cube_shader.setVec3("u_sun_direction", sunDir);
    cube_shader.setFloat("u_shadow_bias", shadowmap._shadow_bias);
    cube_shader.setFloat("u_ambient_occlusion_enabled", _ambient_occlusion);
    cube_shader.setFloat("u_ambient_occlusion_strength", _ambient_occlusion_strength);

    cube_shader.setVec2("u_resolution", glm::vec2(_ctx.width, _ctx.height));
    cube_shader.setFloat("u_sunDotAngle", glm::dot(sunDir, {0.0f, 1.0f, 0.0f}));
    cube_shader.setFloat("u_FOV", glm::radians(camera.fov));

    glBindTextureUnit(0, shadowmap._depthTexture._texture);
    renderTerrain(cube_shader, camera);
    renderEntities(camera);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

    postprocessing_shader.use();
    postprocessing_shader.setVec2("u_resolution", glm::vec2(_ctx.width, _ctx.height));
    postprocessing_shader.setFloat("u_sunDotAngle", glm::dot(sunDir, {0.0f, 1.0f, 0.0f}));
    postprocessing_shader.setFloat("u_FOV", glm::radians(camera.fov));
    postprocessing_shader.setMat4("u_view", camera.getView());
    postprocessing_shader.setMat4("u_projection", camera.getProjection());
    postprocessing_shader.setFloat("u_sunDotAngle", glm::dot(sunDir, {0.0f, 1.0f, 0.0f}));

    postprocessing_shader.setInt("colorTexture", 0);
    postprocessing_shader.setInt("depthTexture", 1);

    glBindTextureUnit(0, _colorTexture._texture);
    glBindTextureUnit(1, _depthTexture._texture);
    _quad_fs.draw();
}

void WorldRenderer::onDeletedChunk(const glm::ivec3 &chunk_pos) {
    const auto& it = meshes.find(chunk_pos);
    if (it == meshes.end()) return;

    buffer_allocator_vertices.deallocate(it->second.slot_vertices.id);
    buffer_allocator_indices.deallocate(it->second.slot_indices.id);
    meshes.erase(it);
}

void WorldRenderer::onAddedChunk(const glm::ivec3 &chunk_pos) {
    for (int z = -1 ; z <= 1; ++z) {
    for (int y = -1 ; y <= 1; ++y) {
    for (int x = -1 ; x <= 1; ++x) {
        const glm::ivec3 offset = {x, y, z};
        chunks_to_remesh.insert(chunk_pos + offset);
    }}}
}

void WorldRenderer::onResize(int width, int height) {
    printf("ON RESIZE\n");

    _framebuffer.destroy();
    _colorTexture.destroy();
    _depthTexture.destroy();

    _framebuffer = Framebuffer();
    _colorTexture = Texture(width, height, GL_RGB8, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
    _depthTexture = Texture(width, height, GL_DEPTH_COMPONENT24, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
    _framebuffer.attachTexture(_colorTexture._texture, GL_COLOR_ATTACHMENT0);
    _framebuffer.attachTexture(_depthTexture._texture, GL_DEPTH_ATTACHMENT);
}

void WorldRenderer::update() {
    processChunksToMesh();
    allocateVAOforWaitingChunks();
}

void WorldRenderer::processChunksToMesh()
{
    for (const auto& pos : chunks_to_remesh) {
        thread_pool.enqueue([this, pos] {
            Chunk* chunk = World::instance().getChunk(pos);
            if (chunk != nullptr) {
                ChunkRawMesh raw_mesh = computeVertexBuffer(pos);

                std::lock_guard<std::mutex> lock(chunks_waiting_bufferslot_mutex);
                chunks_waiting_bufferslot.push_back(std::tuple(pos, raw_mesh));
            }
        });
    }
    chunks_to_remesh.clear();
}

void WorldRenderer::allocateVAOforWaitingChunks() {
    const std::lock_guard<std::mutex> lock(chunks_waiting_bufferslot_mutex);

    for (const auto& [chunk_pos, chunk_raw_mesh]: chunks_waiting_bufferslot) {
        const Chunk* c = World::instance().getChunkUnsafe(chunk_pos);
        if (c == nullptr) continue;

        // find old chunk and delete its vertices
        const auto& it = meshes.find(chunk_pos);
        if (it != meshes.end()) {
            buffer_allocator_vertices.deallocate(it->second.slot_vertices.id);
            buffer_allocator_indices.deallocate(it->second.slot_indices.id);
        }

        ChunkMesh new_mesh;
        new_mesh.updateVAO(buffer_allocator_vertices, buffer_allocator_indices, chunk_raw_mesh);
        meshes[chunk_pos] = new_mesh;
    }

    chunks_waiting_bufferslot.clear();
}


void WorldRenderer::renderTerrain(const Program& program, const Camera &camera, bool use_frustum_culling)
{
    program.use();
    program.setMat4("u_projectionMatrix", camera.getProjection());
    program.setMat4("u_viewMatrix", camera.getView());
    program.setVec3("u_view_position", camera.getPosition());
    program.setFloat("u_time", glfwGetTime());

    Frustum camera_frustum = createFrustumFromCamera(camera, camera.aspect_ratio, glm::radians(camera.fov), camera.near_plane, camera.far_plane);

    chunks_drawn = 0;

    std::vector<DrawElementsIndirectCommand> commands;
    std::vector<glm::vec4> chunk_positions;

    for (const auto& [chunk_pos, mesh] : meshes)
    {
        if (mesh.slot_vertices.id == -1 || mesh.slot_indices.id == -1) continue;

        if (use_frustum_culling) {
            AABB chunk_aabb = {(chunk_pos * 16), (chunk_pos * 16) + 16};
            if (!chunk_aabb.isOnFrustum(camera_frustum)) continue;
        }

        chunk_positions.push_back(glm::vec4(chunk_pos * 16, 1.0f));

        commands.emplace_back(
            mesh.slot_indices.size / sizeof(GLuint),
            1,
            mesh.slot_indices.start / sizeof(GLuint),
            mesh.slot_vertices.start / sizeof(GLuint),
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
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_texture_handles);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_chunk_positions);

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

// void WorldRenderer::renderSkybox(const Camera &camera)
// {
//     glDisable(GL_DEPTH_TEST);
//     skybox_shader.use();
//     skybox_shader.setVec2("u_resolution", glm::vec2(_ctx.width, _ctx.height));
//     skybox_shader.setFloat("u_sunDotAngle", glm::dot(sunDir, {0.0f, 1.0f, 0.0f}));
//     skybox_shader.setFloat("u_FOV", glm::radians(camera.fov));
//     skybox_shader.setMat4("u_view", glm::mat4(glm::mat3(camera.getView())));
//     skybox_shader.setMat4("u_projection", camera.getProjection());

//     skybox_quad.draw();
//     glEnable(GL_DEPTH_TEST);
// }

void WorldRenderer::renderShadowmap(const Camera &camera)
{
    shadowmap.setSunDir(sunDir);
    shadowmap.begin(camera, cube_shadowmapping_shader);
        renderTerrain(cube_shadowmapping_shader, camera, false);
    shadowmap.end();
}
