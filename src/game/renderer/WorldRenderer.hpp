#pragma once

#include "IWorldRender.hpp"
#include "ChunkMesh.hpp"
#include "ShaderProgram.hpp"
#include "Shadowmap.hpp"
#include "Geometry.hpp"
#include "BufferAllocator.hpp"
#include "ThreadPool.hpp"
#include "UniformBuffer.hpp"
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

    glm::vec3 getSunDirection() const;
    void renderEntities(const Camera &camera, const ShaderProgram& program) const;

private:
    float _framebuffer_width;
    float _framebuffer_height;

public:
    Shadowmap shadowmap{4096};
    float _max_shadow_distance = 350.0f;
    float _sun_rotation = glm::radians(90.0f);
    float _sun_pitch = glm::radians(20.0f);
    float _sun_yaw = glm::radians(128.0f);

    FPSCamera _shadow_camera; // camera used for shadow calculations
    bool _is_shadow_camera_freezed = false;
    bool _debug_draw_shadowmap_frustums = false;

    int32_t chunks_drawn;
    bool _wireframe = false;

    Framebuffer _framebuffer;
    Texture _texture_color;
    Texture _texture_world_position;
    Texture _texture_depth;

    Framebuffer _framebuffer_volumetrics;
    Texture _texture_volumetrics;

    GLuint _buffer_ssbo_uniforms;
    // StructGPUBuffer<uniformsParameters> uniform_parameters_buffer;
    uniformsParameters uniform_parameters = {
        .fogDensity = 0.00096f,
        .ambient_occlusion_strength = 0.67f,
        .exposure = 1.0,
        .volumetricDensity = 0.004f,
        .volumetricHGphaseFront = 0.65f,
        .volumetricHGphaseBack = -0.35f,
        .ambient_occlusion_enabled = 1,
        .tonemapping_enabled = 1,
        .cascadeCount = 4,
    };

    Mesh _quad_fs = Geometry::quad_2d();
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

    BufferAllocator buffer_allocator_vertices {"BufferAllocatorVertice", MAX_MEMORY};

    GLuint chunk_vao;
    GLuint draw_command_buffer;
    GLuint ssbo_chunk_positions;
    GLuint ssbo_chunk_element_buffer;

    ThreadPool thread_pool;


    std::unordered_set<glm::ivec3> chunks_to_remesh;
    std::vector<std::tuple<glm::ivec3, ChunkRawMesh>> chunks_waiting_bufferslot;
    std::mutex chunks_waiting_bufferslot_mutex;

    ankerl::unordered_dense::map<glm::ivec3, ChunkMesh> meshes;
};
