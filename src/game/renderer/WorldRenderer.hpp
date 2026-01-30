#pragma once

#include "IWorldRender.hpp"
#include "ChunkMesh.hpp"
#include "ShaderProgram.hpp"
#include "Shadowmap.hpp"
#include "Geometry.hpp"
#include "BufferAllocator.hpp"
#include "ThreadPool.hpp"
// #include "UniformBuffer.hpp"
#include "FpsCamera.hpp"
#include "uniforms_struct.hpp"
#include "unordered_dense.h"
#include <glad/gl.h>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <unordered_set>

// #include "StructGPUBuffer.hpp"

struct ChunkMesh;
struct ChunkRawMesh;
class Camera;

class WorldRenderer : public IWorldRenderer
{
    static constexpr int VERTEX_SIZE = sizeof(GLuint64);

public:
    WorldRenderer(int32_t width, int32_t height);

    void onDeletedChunk(const glm::ivec3& chunk_pos) override;
    void onAddedChunk(const glm::ivec3& chunk_pos) override;
    void onResize(int32_t width, int32_t height) override;

    void render(const Camera &camera) override;
    void update() override;
    void imguiRender() override;

private:
    void processChunksToMesh();
    void allocateVAOforWaitingChunks();

    void setDefaultRenderState();

    void generateDrawCommands(
        std::vector<DrawElementsIndirectCommand>& commands_opaque,
        std::vector<DrawElementsIndirectCommand>& commands_translucent,
        std::vector<glm::vec4>& chunk_positions_opaque,
        std::vector<glm::vec4>& chunk_positions_translucent,
        const glm::mat4 &view_projection,
        bool use_frustum_culling);

    void renderTerrain(
        const std::vector<DrawElementsIndirectCommand>& commands_opaque,
        const std::vector<DrawElementsIndirectCommand>& commands_translucent,
        const std::vector<glm::vec4>& chunk_positions_opaque,
        const std::vector<glm::vec4>& chunk_positions_translucent,
        bool drawTranslucent = true
    );

    void renderEntities(const Camera &camera, const ShaderProgram& program) const;

    glm::vec3 getSunDirection() const;
    glm::quat getSunQuaternionRotation() const;

private:
    float m_framebufferWidth;
    float m_framebufferHeight;

public:
    Shadowmap m_shadowmap{4096};
    GLuint m_textureView[4]{};

    float m_maxShadowDistance = 350.0f;
    float m_sunRotation = glm::radians(90.0f);
    float m_sunPitch = glm::radians(20.0f);
    float m_sunYaw = glm::radians(128.0f);

    FPSCamera m_shadowCamera; // camera used for shadow calculations
    bool m_isShadowCameraFreezed = false;
    bool m_debugDrawShadowmapFrustums = false;

    int32_t m_chunksDrawn;
    bool m_wireframeEnabled = false;

    Framebuffer m_framebuffer;
    Texture m_textureColor;
    Texture m_textureWorldPosition;
    Texture m_textureNormals;
    Texture m_textureDepth;

    Framebuffer m_framebufferVolumetrics;
    Texture m_textureVolumetrics;

    GLuint m_bufferUniformsSSBO;
    // StructGPUBuffer<uniformsParameters> uniform_parameters_buffer;
    uniformsParameters m_uniformParameters = {
        .fogDensity = 0.0005f,
        .ambient_occlusion_strength = 0.67f,
        .exposure = 1.0,
        .volumetricDensity = 0.004f,
        .volumetricHGphaseFront = 0.65f,
        .volumetricHGphaseBack = -0.35f,
        .ambient_occlusion_enabled = 1,
        .tonemapping_enabled = 1,
        .cascadeCount = 4,
        .shadows_enabled = 1,
    };

    // m_meshQuadFullscreen
    Mesh m_quadFS = Geometry::quad_2d();
    Mesh _skybox_cube = Geometry::cube(glm::vec3(1000.0f), glm::vec3(0.0f)); // make it big to avoid clipping with high FOV (>120)

    GLuint ssbo_texture_handles;

    std::unordered_map<std::string, ShaderProgram> _shaders = {
        {"cube",                {RESSOURCE_PATH "shaders/cube.vs",              RESSOURCE_PATH "shaders/cube.fs"            }},
        {"cube_depth_only",     {RESSOURCE_PATH "shaders/cube_depth_only.vs",   RESSOURCE_PATH "shaders/cube_depth_only.fs" }},
        {"mesh",                {RESSOURCE_PATH "shaders/mesh.vs",              RESSOURCE_PATH "shaders/mesh.fs"            }},
        {"postprocessing",      {RESSOURCE_PATH "shaders/texcoords.vs",         RESSOURCE_PATH "shaders/postprocess.fs"     }},
        {"skybox",              {RESSOURCE_PATH "shaders/skybox.vs",            RESSOURCE_PATH "shaders/skybox.fs"          }},
        {"volumetrics",         {RESSOURCE_PATH "shaders/texcoords.vs",         RESSOURCE_PATH "shaders/volumetrics.fs"     }},
        {"bloom_combine",       {RESSOURCE_PATH "shaders/texcoords.vs",         RESSOURCE_PATH "shaders/bloom_combine.fs"   }},
    };

    const int32_t CHUNK_DELETE_DISTANCE_OFFSET = 2;

    const uint32_t MAX_COMMANDS = 500'000;
    const uint32_t MAX_MEMORY = 2'147'483'647;

    BufferAllocator m_bufferAllocatorVertices {"BufferAllocatorVertice", MAX_MEMORY};

    GLuint m_chunkVao;
    GLuint m_drawCommandBuffer;
    GLuint m_ssboChunkPositions;
    GLuint m_ssboChunkElementBuffer;

    ThreadPool m_threadPool;

    std::unordered_set<glm::ivec3> m_chunksToRemesh;
    std::vector<std::tuple<glm::ivec3, ChunkRawMesh>> m_chunksWaitingBufferslot;
    std::mutex m_chunksWaitingBufferslot_mutex;

    ankerl::unordered_dense::map<glm::ivec3, ChunkMesh> m_meshes;
};
