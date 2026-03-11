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
#include "Profiler.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/gtc/quaternion.hpp"
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
    m_chunkVao = createVAO(0, "i");
    m_drawCommandBuffer = createBufferStorage(nullptr, sizeof(DrawElementsIndirectCommand) * MAX_COMMANDS, GL_DYNAMIC_STORAGE_BIT);
    m_ssboChunkPositions = createBufferStorage(nullptr, sizeof(GLfloat)*4 * MAX_COMMANDS, GL_DYNAMIC_STORAGE_BIT);
    m_ssboChunkElementBuffer = createBufferStorage(nullptr, sizeof(uint32_t) * CHUNK_BLOCK_COUNT * 6 * 6, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

    uint32_t* buf = (uint32_t*)glMapNamedBuffer(m_ssboChunkElementBuffer, GL_WRITE_ONLY);
    for (int i = 0 ; i < CHUNK_BLOCK_COUNT * 6 * 6 ; i += 6) {
        buf[i + 0] = i + 0;
        buf[i + 1] = i + 2;
        buf[i + 2] = i + 1;

        buf[i + 3] = i + 0;
        buf[i + 4] = i + 3;
        buf[i + 5] = i + 2;
    }
    glUnmapNamedBuffer(m_ssboChunkElementBuffer);

    _shaders.at("cube").use();
    _shaders.at("cube").setInt("u_shadowmap", 0);

    BlockTextureManager::loadAllTextures();
    ssbo_texture_handles = createBufferStorage(BlockTextureManager::Get().m_texturesHandles.data(), BlockTextureManager::Get().m_texturesHandles.size() * sizeof(GLuint64));

    m_bufferUniformsSSBO = createBufferStorage(nullptr, sizeof(uniformsParameters));

    glGenTextures(m_uniformParameters.cascadeCount, m_textureView);
    for (int i = 0 ; i < m_uniformParameters.cascadeCount ; ++i) {
        glTextureView(
            m_textureView[i], GL_TEXTURE_2D,
            m_shadowmap.m_depthTextureArray, GL_DEPTH_COMPONENT32F,
            0, 1, i, 1
        );
        constexpr GLint rgba[4] = { GL_RED, GL_RED, GL_RED, GL_ONE };
        glTextureParameteriv(m_textureView[i], GL_TEXTURE_SWIZZLE_RGBA, (GLint*)&rgba); // to make the texture grayscale in imgui
    }

    onResize(width, height);
}

void WorldRenderer::setDefaultRenderState()
{
    glEnable(GL_MULTISAMPLE); // enabled by default

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // glEnable(GL_FRAMEBUFFER_SRGB);

    /* reversed-Z */
    glClearDepth(0.0f);
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glDepthFunc(GL_GEQUAL);
}

void WorldRenderer::render(const Camera &camera)
{
    if (!m_isShadowCameraFreezed) {
        m_shadowCamera = dynamic_cast<const FPSCamera &>(camera);
    }

    const glm::mat4 camera_projection = camera.getProjection();

    glm::vec3 cameraPosLocalToChunk = glm::mod(camera.getPosition(), 16.0);
    // glm::mat4 localTranslation = glm::translate(glm::mat4(1.0f), cameraPosLocalToChunk);
    // const glm::mat4 camera_view =  glm::mat4(glm::mat3(m_shadowCamera.getViewLocal()));
    const glm::mat4 camera_view = m_shadowCamera.getViewLocal();
    // const glm::mat4 camera_view = camera.getView();
    const glm::mat4 view_projection = camera_projection * camera_view;


    std::vector<DrawElementsIndirectCommand> commands_opaque;
    std::vector<DrawElementsIndirectCommand> commands_translucent;
    std::vector<glm::vec4> chunk_positions_opaque;
    std::vector<glm::vec4> chunk_positions_translucent;

    const glm::vec3 sunDirection = getSunDirection();
    const glm::quat sunQuat = getSunQuaternionRotation();
    const float sun_dot_angle = glm::dot(glm::normalize(sunDirection), {0.0f, 1.0f, 0.0f});

    m_uniformParameters.projection = camera_projection;
    m_uniformParameters.view = camera_view;
    m_uniformParameters.projection_view = view_projection;
    m_uniformParameters.projection_view_noviewtranslate = camera_projection * glm::mat4(glm::mat3(camera_view));
    m_uniformParameters.resolution = glm::vec2(m_framebufferWidth, m_framebufferHeight);
    m_uniformParameters.aspectRatio = m_framebufferWidth / m_framebufferHeight;
    m_uniformParameters.sunDotAngle = sun_dot_angle;
    m_uniformParameters.FOV = glm::radians(camera.fov);
    m_uniformParameters.sunDirection = glm::vec4(glm::normalize(sunDirection), 0);
    m_uniformParameters.sunQuaternionRotation = glm::vec4(sunQuat.x, sunQuat.y, sunQuat.z, sunQuat.w);
    // m_uniformParameters.viewPosition = glm::vec4(camera.getPosition(), 0);
    m_uniformParameters.viewPosition = glm::vec4(cameraPosLocalToChunk, 0);
    m_uniformParameters.viewDirection = glm::vec4(camera.forward(), 0);
    m_uniformParameters.lightSpaceMatrix = m_shadowmap.m_lightSpaceMatrix;
    m_uniformParameters.shadow_bias = m_shadowmap.m_shadowBias;
    m_uniformParameters.time = (float)glfwGetTime();
    m_uniformParameters.cascadePlaneDistances = *(glm::vec4*)m_shadowmap.shadowCascadeLevels.data();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_bufferUniformsSSBO);

    // uniform_parameters_buffer.set(&uniformsParameters::projection, camera.getProjection());
    // uniform_parameters_buffer.set(&uniformsParameters::view, camera.getView());
    // uniform_parameters_buffer.set(&uniformsParameters::projection_view, view_projection);
    // uniform_parameters_buffer.set(&uniformsParameters::resolution, glm::vec2(_framebuffer_width, _framebuffer_height));
    // uniform_parameters_buffer.set(&uniformsParameters::sunDotAngle, sun_dot_angle);
    // uniform_parameters_buffer.set(&uniformsParameters::FOV, glm::radians(camera.fov));
    // uniform_parameters_buffer.set(&uniformsParameters::sunDirection, glm::vec4(glm::normalize(sunDirection), 0));
    // uniform_parameters_buffer.set(&uniformsParameters::viewPosition, glm::vec4(camera.getPosition(), 0));
    // uniform_parameters_buffer.set(&uniformsParameters::fogDensity, _fog_density);
    // uniform_parameters_buffer.set(&uniformsParameters::lightSpaceMatrix, shadowmap._lightSpaceMatrix);
    // uniform_parameters_buffer.set(&uniformsParameters::shadow_bias, shadowmap._shadow_bias);
    // uniform_parameters_buffer.set(&uniformsParameters::ambient_occlusion_enabled, (int)_ambient_occlusion);
    // uniform_parameters_buffer.set(&uniformsParameters::ambient_occlusion_strength, _ambient_occlusion_strength);
    // uniform_parameters_buffer.set(&uniformsParameters::tonemapping_enabled, (int)_tonemapping);
    // uniform_parameters_buffer.set(&uniformsParameters::time, (float)glfwGetTime());
    // uniform_parameters_buffer.set(&uniformsParameters::exposure, _exposure);
    // uniform_parameters_buffer.bind(3);

    glNamedBufferSubData(m_bufferUniformsSSBO, 0, sizeof(uniformsParameters), &m_uniformParameters);

    setDefaultRenderState();

    if (m_uniformParameters.shadows_enabled)
    { // SHADOWMAP //
        glClearDepth(1.0f);
        glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE );
        glDepthFunc(GL_LESS);

        m_shadowmap.setSunDir(sunDirection);

        const auto lightSpaceMatrices = m_shadowmap.getLightSpaceMatrices(m_shadowCamera);

        std::memcpy(m_uniformParameters.lightSpaceMatrices, lightSpaceMatrices.data(), sizeof(m_uniformParameters.lightSpaceMatrices));
        glNamedBufferSubData(m_bufferUniformsSSBO, 0, sizeof(uniformsParameters), &m_uniformParameters);

        for (int i = 0 ; i < 4 ; ++i) {
            m_shadowmap.begin(lightSpaceMatrices[i], m_shadowCamera.getView(), _shaders.at("cube_depth_only"), i);

            m_uniformParameters.lightSpaceMatrix = lightSpaceMatrices[i];
            glNamedBufferSubData(m_bufferUniformsSSBO, 0, sizeof(uniformsParameters), &m_uniformParameters);

            if (m_debugDrawShadowmapFrustums) {
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

            m_shadowmap.end();
        }

        m_uniformParameters.lightSpaceMatrix = lightSpaceMatrices[0];
        glNamedBufferSubData(m_bufferUniformsSSBO, 0, sizeof(uniformsParameters), &m_uniformParameters);
    }

    m_framebuffer.bind();
    uint32_t attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    m_framebuffer.drawBuffers(3, attachments); // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering

    glClearDepth(0.0f);
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glDepthFunc(GL_GEQUAL);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, m_wireframeEnabled ? GL_LINE : GL_FILL);

    // ZPrePass
    // TODO: should use a framebuffer with glDrawBuffer(GL_NONE) to completely disable fragment stage
    // cube_shader_depth_only.use();
    // cube_shader_depth_only.setMat4("u_lightSpaceMatrix", view_projection);
    // renderTerrain();

    glBindTextureUnit(0, m_shadowmap.m_depthTextureArray);

    {
        ScopedTask("terrain: generateDrawCommands");
        generateDrawCommands(commands_opaque, commands_translucent, chunk_positions_opaque, chunk_positions_translucent, view_projection, true);
    }

    { // skybox (render before terrain for transparent geometry)
        ScopedTaskGPU("skybox: render");

        glDisable(GL_CULL_FACE); // because cube mesh if facing outside
        glDepthMask(GL_FALSE);
        _shaders.at("skybox").use();
        _skybox_cube.draw();
        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
    }

    {
        _shaders.at("cube").use();
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

    // Raymarch volumetric lighting at low res
    {
        ScopedTaskGPU("Volumetrics");

        m_framebufferVolumetrics.bind();
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, m_textureVolumetrics._width, m_textureVolumetrics._height);

        glBindTextureUnit(1, m_textureWorldPosition._texture);
        glBindTextureUnit(3, m_shadowmap.m_depthTextureArray);

        const auto& shader_volumetrics = _shaders.at("volumetrics");
        shader_volumetrics.use();
        shader_volumetrics.setInt("worldPosTexture", 1);
        shader_volumetrics.setInt("u_shadowmap", 3);

        m_quadFS.draw();
    }

    glViewport(0, 0, m_framebufferWidth, m_framebufferHeight);

    // - Extract bright areas
    // - Combine bright areas with volumetrics
    // - Apply bloom

    // Combine Volumetrics with scene
    {
        ScopedTaskGPU("Combine Volumetrics");

        m_framebuffer.bind();
        m_framebuffer.drawBuffers(1, attachments);

        const auto& shader_combine = _shaders.at("bloom_combine");
        shader_combine.use();
        shader_combine.setInt("u_scene", 0);
        shader_combine.setInt("u_bloomBlur", 1);

        glBindTextureUnit(0, m_textureColor._texture);
        glBindTextureUnit(1, m_textureVolumetrics._texture);

        m_quadFS.draw();
    }


    // Post Processing
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // disable wires mode
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

        const auto& shader_post_processing = _shaders.at("postprocessing");
        shader_post_processing.use();
        shader_post_processing.setInt("colorTexture", 0);
        // shader_post_processing.setInt("depthTexture", 2);

        glBindTextureUnit(0, m_textureColor._texture);
        // glBindTextureUnit(2, _depth_texture._texture);

        {
            ScopedTaskGPU("postProcessing");
            m_quadFS.draw();
        }
    }
}

void WorldRenderer::onDeletedChunk(const glm::ivec3 &chunk_pos) {
    const auto& it = m_meshes.find(chunk_pos);
    if (it == m_meshes.end()) return;

    m_bufferAllocatorVertices.deallocate(it->second.slot_vertices);
    m_bufferAllocatorVertices.deallocate(it->second.slot_vertices_translucent);
    m_meshes.erase(it);
}

void WorldRenderer::onAddedChunk(const glm::ivec3 &chunk_pos) {
    for (int32_t z = -1 ; z <= 1; ++z) {
    for (int32_t y = -1 ; y <= 1; ++y) {
    for (int32_t x = -1 ; x <= 1; ++x) {
        const glm::ivec3 offset = {x, y, z};
        m_chunksToRemesh.insert(chunk_pos + offset);
    }}}
}

void WorldRenderer::onResize(int32_t width, int32_t height) {
    width = glm::max(8, width);
    height = glm::max(8, height);

    m_framebufferWidth = width;
    m_framebufferHeight = height;

    m_framebuffer.destroy();
    m_textureColor.destroy();
    m_textureWorldPosition.destroy();
    m_textureNormals.destroy();
    m_textureDepth.destroy();
    m_framebufferVolumetrics.destroy();
    m_textureVolumetrics.destroy();

    m_framebuffer = Framebuffer();
    m_textureColor = Texture(width, height, GL_RGB16F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
    m_textureWorldPosition = Texture(width, height, GL_RGB32F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
    m_textureNormals = Texture(width, height, GL_RGB32F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
    m_textureDepth = Texture(width, height, GL_DEPTH_COMPONENT32F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER); // floating point buffer needed for reversed depth
    m_framebuffer.attachTexture(m_textureColor._texture, GL_COLOR_ATTACHMENT0);
    m_framebuffer.attachTexture(m_textureWorldPosition._texture, GL_COLOR_ATTACHMENT1);
    m_framebuffer.attachTexture(m_textureNormals._texture, GL_COLOR_ATTACHMENT2);
    m_framebuffer.attachTexture(m_textureDepth._texture, GL_DEPTH_ATTACHMENT);

    m_framebufferVolumetrics = Framebuffer();
    m_textureVolumetrics = Texture(width/4, height/4, GL_RGB16F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
    m_framebufferVolumetrics.attachTexture(m_textureVolumetrics._texture, GL_COLOR_ATTACHMENT0);
}

void WorldRenderer::update() {
    processChunksToMesh();
    allocateVAOforWaitingChunks();
}

void WorldRenderer::processChunksToMesh()
{
    for (const auto& pos : m_chunksToRemesh) {
        m_threadPool.enqueue([this, pos] {
            Chunk* chunk = World::instance().getChunk(pos);
            if (chunk != nullptr) {
                ChunkRawMesh raw_mesh = computeVertexBuffer(pos);

                std::lock_guard<std::mutex> lock(m_chunksWaitingBufferslot_mutex);
                m_chunksWaitingBufferslot.push_back(std::tuple(pos, raw_mesh));
            }
        });
    }
    m_chunksToRemesh.clear();
}

void WorldRenderer::allocateVAOforWaitingChunks() {
    const std::lock_guard<std::mutex> lock(m_chunksWaitingBufferslot_mutex);

    for (const auto& [chunk_pos, chunk_raw_mesh]: m_chunksWaitingBufferslot) {
        const Chunk* c = World::instance().getChunkUnsafe(chunk_pos);
        if (c == nullptr) continue;

        // find old chunk and delete its vertices
        const auto& it = m_meshes.find(chunk_pos);
        if (it != m_meshes.end()) {
            m_bufferAllocatorVertices.deallocate(it->second.slot_vertices);
            m_bufferAllocatorVertices.deallocate(it->second.slot_vertices_translucent);
        }

        ChunkMesh new_mesh;
        new_mesh.updateVAO(m_bufferAllocatorVertices, chunk_raw_mesh);
        m_meshes[chunk_pos] = new_mesh;
    }

    m_chunksWaitingBufferslot.clear();
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

    m_chunksDrawn = 0;

    glm::dvec3 cameraPos = m_shadowCamera.getPosition();
    glm::ivec3 cameraPosSnapped = glm::floor(cameraPos / 16.0) * 16.0;

    for (const auto& [chunk_pos, mesh] : m_meshes)
    {
        if (mesh.slot_vertices.start == -1 && mesh.slot_vertices_translucent.start == -1) continue;

        // if (use_frustum_culling) {
        //     AABB chunk_aabb = {(chunk_pos * CHUNK_SIZE), (chunk_pos * CHUNK_SIZE) + CHUNK_SIZE};
        //     if (!isAABBOnFrustum(chunk_aabb, camera_frustum)) continue;
        // }

        glm::ivec3 chunkPosMoved = chunk_pos - cameraPosSnapped/16;

        if (mesh.slot_vertices.start != -1) {
            chunk_positions_opaque.push_back(glm::vec4(chunkPosMoved * CHUNK_SIZE, 1.0f));
            commands_opaque.push_back({
                (uint32_t)(mesh.slot_vertices.size / VERTEX_SIZE) * 6, // one face if 2 triangles, 6 vertices
                1u,
                0u,
                0, // don't need it first vertex so set it at 0 to avoid crash/bug
                (uint32_t)(mesh.slot_vertices.start / VERTEX_SIZE), // pass first vertex information by using this field that get sent to gl_BaseInstance
            });
        }

        if (mesh.slot_vertices_translucent.start != -1) {
            chunk_positions_translucent.push_back(glm::vec4(chunkPosMoved * CHUNK_SIZE, 1.0f));
            commands_translucent.push_back({
                (uint32_t)(mesh.slot_vertices_translucent.size / VERTEX_SIZE) * 6, // one face if 2 triangles, 6 vertices
                1u,
                0u,
                0, // don't need it first vertex so set it at 0 to avoid crash/bug
                (uint32_t)(mesh.slot_vertices_translucent.start / VERTEX_SIZE), // pass first vertex information by using this field that get sent to gl_BaseInstance
            });
        }

        ++m_chunksDrawn;
    }
}

void WorldRenderer::renderTerrain(
    const std::vector<DrawElementsIndirectCommand>& commands_opaque,
    const std::vector<DrawElementsIndirectCommand>& commands_translucent,
    const std::vector<glm::vec4>& chunk_positions_opaque,
    const std::vector<glm::vec4>& chunk_positions_translucent,
    bool drawTranslucent
) {
    glBindVertexArray(m_chunkVao);
    glVertexArrayVertexBuffer(m_chunkVao, 0, m_bufferAllocatorVertices.getBufferObject(), 0, 1 * VERTEX_SIZE); // Not needed anymore but crashes without
    glVertexArrayElementBuffer(m_chunkVao, m_ssboChunkElementBuffer);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_texture_handles);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssboChunkPositions);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bufferAllocatorVertices.getBufferObject());

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_drawCommandBuffer);

    // opaque
    glNamedBufferSubData(m_ssboChunkPositions, 0, sizeof(GLfloat) * 4 * chunk_positions_opaque.size(), (const void *)chunk_positions_opaque.data());
    glNamedBufferSubData(m_drawCommandBuffer, 0, sizeof(commands_opaque[0]) * commands_opaque.size(), (const void *)commands_opaque.data());
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void *)0, commands_opaque.size(), 0);

    // translucent //
    if (!drawTranslucent) return;
    glNamedBufferSubData(m_ssboChunkPositions, 0, sizeof(GLfloat) * 4 * chunk_positions_translucent.size(), (const void *)chunk_positions_translucent.data());
    glNamedBufferSubData(m_drawCommandBuffer, 0, sizeof(commands_translucent[0]) * commands_translucent.size(), (const void *)commands_translucent.data());

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

    for (const auto& entity : World::instance().m_entities) {
        program.setMat4("u_modelMatrix", entity.smoothTransform.getMatrix());
        entity.draw();
    }
}

glm::vec3 WorldRenderer::getSunDirection() const
{
    glm::vec3 v = glm::normalize(
        glm::vec3(
            glm::yawPitchRoll(m_sunYaw, m_sunPitch, m_sunRotation) * glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}
        )
    );

    return v;
}

glm::quat WorldRenderer::getSunQuaternionRotation() const
{
    return glm::quat_cast(glm::yawPitchRoll(m_sunYaw, m_sunPitch, m_sunRotation));
}

// void WorldRenderer::imguiRender()
// {
//     static bool _gbuffer_window = false;
//     ImGui::Checkbox("G-Buffer window", &_gbuffer_window);
//     if (_gbuffer_window) {
//         ImGui::Begin("G-buffer");
//             ImGui::BeginGroup();
//                 ImGui::Text("Volumetrics");
//                 ImGui::Image((ImTextureID)(intptr_t) world_renderer.m_textureVolumetrics._texture, ImVec2(world_renderer.m_textureVolumetrics._width/1, world_renderer.m_textureVolumetrics._height/1), ImVec2(0, 1), ImVec2(1, 0));
//             ImGui::EndGroup();
//             ImGui::SameLine();
//             ImGui::BeginGroup();
//                 ImGui::Text("World Position");
//                 ImGui::Image((ImTextureID)(intptr_t) world_renderer._texture_world_position._texture, ImVec2(world_renderer._texture_world_position._width/4, world_renderer._texture_world_position._height/4), ImVec2(0, 1), ImVec2(1, 0));
//             ImGui::EndGroup();
//             ImGui::BeginGroup();
//                 ImGui::Text("Normals");
//                 ImGui::Image((ImTextureID)(intptr_t) world_renderer._texture_normals._texture, ImVec2(world_renderer._texture_normals._width/4, world_renderer._texture_normals._height/4), ImVec2(0, 1), ImVec2(1, 0));
//             ImGui::EndGroup();
//         ImGui::End();
//     }
// }
