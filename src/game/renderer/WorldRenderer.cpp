#include "WorldRenderer.hpp"
#include "Frustum.hpp"
#include "World.hpp"
#include "Chunk.hpp"
#include "camera/Camera.hpp"
#include "AABB.hpp"
#include "BlockTextureManager.hpp"
#include "DebugDraw.hpp"
#include "VAO.hpp"
#include "ChunkMesh.hpp"
#include "UniformBuffer.hpp"
#include "Profiler.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>

inline double nsToMs(int64_t ns) {
    return double(ns) / 1e6;
}

inline double nsToS(int64_t ns) {
    return double(ns) / 1e9;
}

WorldRenderer::WorldRenderer(int32_t width, int32_t height)
{
    chunk_vao = createVAO(0, "i");
    draw_command_buffer = createBufferStorage(nullptr, sizeof(DrawElementsIndirectCommand) * MAX_COMMANDS, GL_DYNAMIC_STORAGE_BIT);
    ssbo_chunk_positions = createBufferStorage(nullptr, sizeof(GLfloat)*4 * MAX_COMMANDS, GL_DYNAMIC_STORAGE_BIT);
    ssbo_chunk_element_buffer = createBufferStorage(nullptr, sizeof(uint32_t) * CHUNK_BLOCK_COUNT * 6 * 6, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

    uint32_t* buf = (uint32_t*)glMapNamedBuffer(ssbo_chunk_element_buffer, GL_WRITE_ONLY);
    for (int i = 0 ; i < CHUNK_BLOCK_COUNT * 6 * 6 ; i += 6) {
        buf[i + 0] = i + 0;
        buf[i + 1] = i + 2;
        buf[i + 2] = i + 1;

        buf[i + 3] = i + 0;
        buf[i + 4] = i + 3;
        buf[i + 5] = i + 2;
    }
    glUnmapNamedBuffer(ssbo_chunk_element_buffer);

    _shaders.at("cube").use();
    _shaders.at("cube").setInt("u_shadowmap", 0);

    BlockTextureManager::loadAllTextures();
    ssbo_texture_handles = createBufferStorage(BlockTextureManager::Get().textures_handles.data(), BlockTextureManager::Get().textures_handles.size() * sizeof(GLuint64));

    _ubuffer.makeBufferDef({
        {"projection",                    sizeof(float)*16},
        {"view",                          sizeof(float)*16},
        {"projection_view",               sizeof(float)*16},
        {"lightSpaceMatrix",              sizeof(float)*16},
        {"sunDirection",                  sizeof(float)*4},
        {"viewPosition",                  sizeof(float)*4},
        {"resolution",                    sizeof(float)*2},
        {"sunDotAngle",                   sizeof(float)*1},
        {"FOV",                           sizeof(float)*1},
        {"fogDensity",                    sizeof(float)*1},
        {"shadow_bias",                   sizeof(float)*1},
        {"ambient_occlusion_strength",    sizeof(float)*1},
        {"time",                          sizeof(float)*1},
        {"exposure",                      sizeof(float)*1},
        {"ambient_occlusion_enabled",     sizeof(int)*1},
        {"tonemapping_enabled",           sizeof(int)*1},
    });
    _ubuffer.bind(0);

    _ubuffer_matrices = createBufferStorage(nullptr, 4*16 * 4);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, _ubuffer_matrices);

    // DEBUG strides
    // for (const auto &[name, info] : _ubuffer._uniforms) {
    //     auto ix = glGetProgramResourceIndex(cube_shader.ID, GL_UNIFORM, (std::string("uniformBuffer.") + name).c_str());
    //     GLenum props[] = {GL_ARRAY_STRIDE, GL_OFFSET};
    //     GLint values[2] = {};
    //     glGetProgramResourceiv(cube_shader.ID, GL_UNIFORM, ix, 2, props, 2, NULL, values);

    //     logD("{}: {} {}", name, values[0], values[1]);
    //     auto byteOffset = values[1] + (3 * values[0]);
    // }

    onResize(width, height);
}

void WorldRenderer::setDefaultRenderState()
{
    glEnable(GL_MULTISAMPLE); // enabled by default

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // glEnable(GL_FRAMEBUFFER_SRGB);
}

void WorldRenderer::render(const Camera &camera)
{
    const glm::mat4 view_projection = camera.getProjection() * camera.getView();
    if (!_is_shadow_camera_freezed) {
        _shadow_camera = dynamic_cast<const FPSCamera &>(camera);
    }

    std::vector<DrawElementsIndirectCommand> commands_opaque;
    std::vector<DrawElementsIndirectCommand> commands_translucent;
    std::vector<glm::vec4> chunk_positions_opaque;
    std::vector<glm::vec4> chunk_positions_translucent;

    const glm::vec3 sunDirection = getSunDirection();

    const float sun_dot_angle = glm::dot(glm::normalize(sunDirection), {0.0f, 1.0f, 0.0f});

    _ubuffer.set("projection", camera.getProjection());
    _ubuffer.set("view", camera.getView());
    _ubuffer.set("projection_view", view_projection);
    _ubuffer.set("resolution", glm::vec2(_framebuffer_width, _framebuffer_height));
    _ubuffer.set("sunDotAngle", sun_dot_angle);
    _ubuffer.set("FOV", glm::radians(camera.fov));
    _ubuffer.set("sunDirection", glm::vec4(glm::normalize(sunDirection), 0));
    _ubuffer.set("viewPosition", glm::vec4(camera.getPosition(), 0));
    _ubuffer.set("fogDensity", _fog_density);
    _ubuffer.set("lightSpaceMatrix", shadowmap._lightSpaceMatrix);
    _ubuffer.set("shadow_bias", shadowmap._shadow_bias);
    _ubuffer.set("ambient_occlusion_enabled", (int)_ambient_occlusion);
    _ubuffer.set("ambient_occlusion_strength", _ambient_occlusion_strength);
    _ubuffer.set("tonemapping_enabled", (int)_tonemapping);
    _ubuffer.set("time", (float)glfwGetTime());
    _ubuffer.set("exposure", _exposure);

    setDefaultRenderState();

    { // SHADOWMAP //
        shadowmap.setSunDir(sunDirection);

        const auto lightSpaceMatrices = shadowmap.getLightSpaceMatrices(_shadow_camera);

        glNamedBufferSubData(_ubuffer_matrices, 0, 4*16 * 4, lightSpaceMatrices.data());

        for (int i = 0 ; i < 4 ; ++i) {
            // const glm::mat4 camera_projection_shorter = glm::perspective(glm::radians(camera.fov), camera.aspect_ratio, 0.1f, 1000.0f);
            // const glm::mat4 camera_projection_shorter = glm::perspective(glm::radians(camera.fov), camera.aspect_ratio, 0.1f, _max_shadow_distance);

            // const glm::mat4 camera_projection_shorter = glm::perspective(glm::radians(camera.fov), camera.aspect_ratio, 0.1f, shadowmap.shadowCascadeLevels[i]);
            // glm::mat4 light_space_matrix = shadowmap.begin(lightSpaceMatrices[i], camera.getView(), _shaders.at("cube_depth_only"), i);
            shadowmap.begin(lightSpaceMatrices[i], _shadow_camera.getView(), _shaders.at("cube_depth_only"), i);
            _ubuffer.set("lightSpaceMatrix", lightSpaceMatrices[i]);

            if (_debug_draw_shadowmap_frustums) {
                constexpr glm::vec3 debug_colors[4] = {{1,0,0}, {0,1,0}, {0,0,1}, {1,0,1}};
                DebugDraw::instance().drawFrustum(lightSpaceMatrices[i], debug_colors[i]);
            }

            {
                ScopedTask(std::string("shadowmap: generateDrawCommands ") + std::to_string(i));
                generateDrawCommands(commands_opaque, commands_translucent, chunk_positions_opaque, chunk_positions_translucent, lightSpaceMatrices[i], true);
            }
            glDisable(GL_CULL_FACE);
            {
                ScopedTaskGPU(std::string("shadowmap: render ") + std::to_string(i));
                renderTerrain(commands_opaque, commands_translucent, chunk_positions_opaque, chunk_positions_translucent, false);
            }
            glEnable(GL_CULL_FACE);

            commands_opaque.clear();
            commands_translucent.clear();
            chunk_positions_opaque.clear();
            chunk_positions_translucent.clear();

            shadowmap.end();
        }

        _ubuffer.set("lightSpaceMatrix", lightSpaceMatrices[0]);
        // glNamedFramebufferTextureLayer(shadowmap._depthFBO._framebuffer, GL_DEPTH_ATTACHMENT, shadowmap._depthTextureArray, 0, 0);
    }


    _framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);

    // ZPrePass
    // TODO: should use a framebuffer with glDrawBuffer(GL_NONE) to completely disable fragment stage
    // cube_shader_depth_only.use();
    // cube_shader_depth_only.setMat4("u_lightSpaceMatrix", view_projection);
    // renderTerrain(camera.getProjection() * camera.getView(), true);

    // glBindTextureUnit(0, shadowmap._depthTexture._texture);
    glBindTextureUnit(0, shadowmap._depthTextureArray);

    uint32_t attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments); // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering

    { // skybox
        ScopedTaskGPU("skybox: render");

        glDisable(GL_CULL_FACE); // because cube mesh if facing outside
        glDepthMask(GL_FALSE);

        _shaders.at("skybox").use();
        _shaders.at("skybox").setMat4("u_view", glm::mat4(glm::mat3(camera.getView())));
        _shaders.at("skybox").setMat4("u_projection", camera.getProjection());
        _skybox_cube.draw();

        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
    }

    {
        ScopedTask("terrain: generateDrawCommands");
        generateDrawCommands(commands_opaque, commands_translucent, chunk_positions_opaque, chunk_positions_translucent, view_projection, true);
    }

    _shaders.at("cube").use();

    _shaders.at("cube").setFloat("u_cascadePlaneDistances[0]", shadowmap.shadowCascadeLevels[0]);
    _shaders.at("cube").setFloat("u_cascadePlaneDistances[1]", shadowmap.shadowCascadeLevels[1]);
    _shaders.at("cube").setFloat("u_cascadePlaneDistances[2]", shadowmap.shadowCascadeLevels[2]);
    _shaders.at("cube").setFloat("u_cascadePlaneDistances[3]", shadowmap.shadowCascadeLevels[3]);

    {
        // glDepthFunc(GL_EQUAL); // used for depth prepass
        ScopedTaskGPU("terrain: render");
        renderTerrain(commands_opaque, commands_translucent, chunk_positions_opaque, chunk_positions_translucent);
        // glDepthFunc(GL_LESS); // used for depth prepass
    }


    {
        ScopedTaskGPU("entities");
        renderEntities(camera, _shaders.at("mesh"));
    }


    DebugDraw::instance().drawAndFlush(view_projection);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // disable wires mode
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.


    const auto& shader_post_processing = _shaders.at("postprocessing");
    shader_post_processing.use();
    shader_post_processing.setInt("colorTexture", 0);
    shader_post_processing.setInt("worldPosTexture", 1);
    // shader_post_processing.setInt("depthTexture", 2);
    shader_post_processing.setInt("u_shadowmap", 3);

    shader_post_processing.setFloat("u_cascadePlaneDistances[0]", shadowmap.shadowCascadeLevels[0]);
    shader_post_processing.setFloat("u_cascadePlaneDistances[1]", shadowmap.shadowCascadeLevels[1]);
    shader_post_processing.setFloat("u_cascadePlaneDistances[2]", shadowmap.shadowCascadeLevels[2]);
    shader_post_processing.setFloat("u_cascadePlaneDistances[3]", shadowmap.shadowCascadeLevels[3]);

    shader_post_processing.setFloat("test_slider_0", test_slider_0);
    shader_post_processing.setFloat("test_slider_1", test_slider_1);
    shader_post_processing.setFloat("test_slider_2", test_slider_2);

    glBindTextureUnit(0, _texture_color._texture);
    glBindTextureUnit(1, _texture_world_position._texture);
    // glBindTextureUnit(2, _depth_texture._texture);
    glBindTextureUnit(3, shadowmap._depthTextureArray);

    {
        ScopedTaskGPU("postProcessing");
        _quad_fs.draw();
    }
}

void WorldRenderer::onDeletedChunk(const glm::ivec3 &chunk_pos) {
    const auto& it = meshes.find(chunk_pos);
    if (it == meshes.end()) return;

    buffer_allocator_vertices.deallocate(it->second.slot_vertices);
    buffer_allocator_vertices.deallocate(it->second.slot_vertices_translucent);
    meshes.erase(it);
}

void WorldRenderer::onAddedChunk(const glm::ivec3 &chunk_pos) {
    for (int32_t z = -1 ; z <= 1; ++z) {
    for (int32_t y = -1 ; y <= 1; ++y) {
    for (int32_t x = -1 ; x <= 1; ++x) {
        const glm::ivec3 offset = {x, y, z};
        chunks_to_remesh.insert(chunk_pos + offset);
    }}}
}

void WorldRenderer::onResize(int32_t width, int32_t height) {
    _framebuffer_width = width;
    _framebuffer_height = height;

    _framebuffer.destroy();
    _texture_color.destroy();
    _texture_world_position.destroy();
    _texture_depth.destroy();

    _framebuffer = Framebuffer();
    _texture_color = Texture(width, height, GL_RGB8, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
    _texture_world_position = Texture(width, height, GL_RGB32F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
    _texture_depth = Texture(width, height, GL_DEPTH_COMPONENT24, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
    _framebuffer.attachTexture(_texture_color._texture, GL_COLOR_ATTACHMENT0);
    _framebuffer.attachTexture(_texture_world_position._texture, GL_COLOR_ATTACHMENT1);
    _framebuffer.attachTexture(_texture_depth._texture, GL_DEPTH_ATTACHMENT);
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
            buffer_allocator_vertices.deallocate(it->second.slot_vertices);
            buffer_allocator_vertices.deallocate(it->second.slot_vertices_translucent);
        }

        ChunkMesh new_mesh;
        new_mesh.updateVAO(buffer_allocator_vertices, chunk_raw_mesh);
        meshes[chunk_pos] = new_mesh;
    }

    chunks_waiting_bufferslot.clear();
}

void WorldRenderer::generateDrawCommands(
    std::vector<DrawElementsIndirectCommand>& commands_opaque,
    std::vector<DrawElementsIndirectCommand>& commands_translucent,
    std::vector<glm::vec4>& chunk_positions_opaque,
    std::vector<glm::vec4>& chunk_positions_translucent,
    const glm::mat4 &view_projection,
    bool use_frustum_culling
) {
    Frustum camera_frustum = createFrustumFromViewProjection(view_projection);

    chunks_drawn = 0;

    for (const auto& [chunk_pos, mesh] : meshes)
    {
        if (mesh.slot_vertices.start == -1 && mesh.slot_vertices_translucent.start == -1) continue;

        if (use_frustum_culling) {
            AABB chunk_aabb = {(chunk_pos * CHUNK_SIZE), (chunk_pos * CHUNK_SIZE) + CHUNK_SIZE};
            if (!isAABBOnFrustum(chunk_aabb, camera_frustum)) continue;
        }

        if (mesh.slot_vertices.start != -1) {
            chunk_positions_opaque.push_back(glm::vec4(chunk_pos * CHUNK_SIZE, 1.0f));
            commands_opaque.push_back({
                (uint32_t)(mesh.slot_vertices.size / VERTEX_SIZE) * 6, // one face if 2 triangles, 6 vertices
                1u,
                0u,
                0, // don't need it first vertex so set it at 0 to avoid crash/bug
                (uint32_t)(mesh.slot_vertices.start / VERTEX_SIZE), // pass first vertex information by using this field that get sent to gl_BaseInstance
            });
        }

        if (mesh.slot_vertices_translucent.start != -1) {
            chunk_positions_translucent.push_back(glm::vec4(chunk_pos * CHUNK_SIZE, 1.0f));
            commands_translucent.push_back({
                (uint32_t)(mesh.slot_vertices_translucent.size / VERTEX_SIZE) * 6, // one face if 2 triangles, 6 vertices
                1u,
                0u,
                0, // don't need it first vertex so set it at 0 to avoid crash/bug
                (uint32_t)(mesh.slot_vertices_translucent.start / VERTEX_SIZE), // pass first vertex information by using this field that get sent to gl_BaseInstance
            });
        }

        ++chunks_drawn;
    }
}

void WorldRenderer::renderTerrain(
    const std::vector<DrawElementsIndirectCommand>& commands_opaque,
    const std::vector<DrawElementsIndirectCommand>& commands_translucent,
    const std::vector<glm::vec4>& chunk_positions_opaque,
    const std::vector<glm::vec4>& chunk_positions_translucent,
    bool drawTranslucent
) {
    glBindVertexArray(chunk_vao);
    glVertexArrayVertexBuffer(chunk_vao, 0, buffer_allocator_vertices.getBufferObject(), 0, 1 * VERTEX_SIZE); // Not needed anymore but crashes without
    glVertexArrayElementBuffer(chunk_vao, ssbo_chunk_element_buffer);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_texture_handles);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_chunk_positions);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffer_allocator_vertices.getBufferObject());

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_command_buffer);

    // opaque
    glNamedBufferSubData(ssbo_chunk_positions, 0, sizeof(GLfloat) * 4 * chunk_positions_opaque.size(), (const void *)chunk_positions_opaque.data());
    glNamedBufferSubData(draw_command_buffer, 0, sizeof(commands_opaque[0]) * commands_opaque.size(), (const void *)commands_opaque.data());
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void *)0, commands_opaque.size(), 0);

    // translucent //
    if (!drawTranslucent) return;
    glNamedBufferSubData(ssbo_chunk_positions, 0, sizeof(GLfloat) * 4 * chunk_positions_translucent.size(), (const void *)chunk_positions_translucent.data());
    glNamedBufferSubData(draw_command_buffer, 0, sizeof(commands_translucent[0]) * commands_translucent.size(), (const void *)commands_translucent.data());

    glEnable(GL_BLEND);
    // glDisable(GL_DEPTH_TEST);
    // glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void *)0, commands_translucent.size(), 0);
    glDisable(GL_BLEND);
    // glEnable(GL_DEPTH_TEST);
    // glDepthMask(GL_TRUE);
}

void WorldRenderer::renderEntities(const Camera &camera, const ShaderProgram& program) const
{
    program.use();
    program.setMat4("u_projectionMatrix", camera.getProjection());
    program.setMat4("u_viewMatrix", camera.getView());

    for (const auto& entity : World::instance().entities) {
        program.setMat4("u_modelMatrix", entity.smooth_transform.getMatrix());
        entity.draw();
    }
}

glm::vec3 WorldRenderer::getSunDirection() const
{
    return glm::normalize(
        glm::yawPitchRoll(_sun_yaw, _sun_pitch, _sun_rotation)
        * glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}
    );
}
