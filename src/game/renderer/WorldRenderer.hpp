#pragma once

#include <unordered_set>

#include <glad/gl.h>
#include <glm/gtx/hash.hpp>

#include "ChunkMesh.hpp"
#include "Context.hpp"
#include "Program.hpp"
#include "Shadowmap.hpp"
#include "Geometry.hpp"
#include "BufferAllocator.hpp"
#include "ThreadPool.hpp"
#include "UniformBuffer.hpp"

struct ChunkMesh;
struct ChunkRawMesh;
class Camera;

class WorldRenderer
{
    using VERTEX_TYPE = GLuint64;

public:
    WorldRenderer(Context &context);

    void onDeletedChunk(const glm::ivec3& chunk_pos);
    void onAddedChunk(const glm::ivec3& chunk_pos);
    void onResize(int32_t width, int32_t height);

    void render(const Camera &camera);
    void update();

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
        const std::vector<glm::vec4>& chunk_positions_translucent
    );

    void renderEntities(const Camera &camera, const Program& program);

    void renderShadowmap(const Camera &camera);

private:
    Context &_ctx;

public:
    Shadowmap shadowmap{_ctx, 4096, 4096};
    float _max_shadow_distance = 350.0f;

    glm::vec3 sunDir = glm::normalize(glm::vec3(20.0f, 50.0f, 20.0f));
    int32_t chunks_drawn;
    bool _wireframe = false;
    bool _ambient_occlusion = true;
    float _ambient_occlusion_strength = 0.67;
    float _fog_density = 0.00096f;
    bool _tonemapping = true;
    float _exposure = 1.0f;

    Framebuffer _framebuffer;
    Texture _color_texture;
    Texture _world_position_texture;
    Texture _depth_texture;

    UniformBuffer _ubuffer;

    Mesh _quad_fs = Geometry::quad_2d();

    GLuint ssbo_texture_handles;

    Program cube_shader                 {RESSOURCE_PATH "shaders/cube.vs",            RESSOURCE_PATH "shaders/cube.fs"};
    Program cube_shader_depth_only      {RESSOURCE_PATH "shaders/cube_depth_only.vs", RESSOURCE_PATH "shaders/cube_depth_only.fs"};
    Program mesh_shader                 {RESSOURCE_PATH "shaders/mesh.vs",            RESSOURCE_PATH "shaders/mesh.fs"};
    Program postprocessing_shader       {RESSOURCE_PATH "shaders/postprocess.vs",     RESSOURCE_PATH "shaders/postprocess.fs"};

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

    std::unordered_map<glm::ivec3, ChunkMesh> meshes;
};
