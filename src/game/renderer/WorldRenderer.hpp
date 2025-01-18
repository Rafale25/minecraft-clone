#pragma once

#include "glad/gl.h"

#include "Context.hpp"
#include "Program.h"
#include "Shadowmap.hpp"

#include "Geometry.hpp"
#include "BufferAllocator.hpp"

#include "ChunkMesh.hpp"
#include "glm/gtx/hash.hpp"

#include "ThreadPool.h"

#include <unordered_set>

class Camera;

class WorldRenderer
{
public:
    WorldRenderer(Context &context);

    void onDeletedChunk(const glm::ivec3& chunk_pos);
    void onAddedChunk(const glm::ivec3& chunk_pos);
    void onResize(int width, int height);

    void render(const Camera &camera);
    void update();

private:
    void processChunksToMesh();
    void allocateVAOforWaitingChunks();

    void setDefaultRenderState();

    void renderTerrain(const Program& program, const Camera& camera, bool use_frustum_culling=true);

    void renderEntities(const Camera &camera);
    void renderEntitiesDepth(const Camera &camera);

    // void renderSkybox(const Camera &camera);
    void renderShadowmap(const Camera &camera);

private:
    Context &_ctx;

public:
    Shadowmap shadowmap{_ctx, 4096, 4096};
    glm::vec3 sunDir = glm::normalize(glm::vec3(20.0f, 50.0f, 20.0f));
    int chunks_drawn;
    bool _wireframe = false;
    bool _ambient_occlusion = true;
    float _ambient_occlusion_strength = 0.67;

    Framebuffer _framebuffer;
    Texture _colorTexture;
    Texture _depthTexture;

    Mesh _quad_fs = Geometry::quad_2d();

    GLuint ssbo_texture_handles;

    Program cube_shader                 {"./assets/shaders/cube.vs", "./assets/shaders/cube.fs"};
    Program cube_shader_depth_only      {"./assets/shaders/cube_depth_only.vs", "./assets/shaders/cube_depth_only.fs"};
    Program mesh_shader                 {"./assets/shaders/mesh.vs", "./assets/shaders/mesh.fs"};
    Program postprocessing_shader       {"./assets/shaders/postprocess.vs", "./assets/shaders/postprocess.fs"};

    const float chunk_view_distance = 16.0f * 16.0f;
    const float chunk_delete_offset = 16.0f * 16.0f;

    const uint32_t MAX_COMMANDS = 16'000;
    BufferAllocator buffer_allocator_vertices {"BufferAllocatorVertice", 25'000 * sizeof(int), MAX_COMMANDS};
    BufferAllocator buffer_allocator_indices  {"BufferAllocatorIndices", 25'000 * sizeof(int), MAX_COMMANDS};

    GLuint chunk_vao;
    GLuint draw_command_buffer;
    GLuint ssbo_chunk_positions;

    ThreadPool thread_pool;

    std::unordered_set<glm::ivec3> chunks_to_remesh;
    std::vector<std::tuple<glm::ivec3, ChunkRawMesh>> chunks_waiting_bufferslot;
    std::mutex chunks_waiting_bufferslot_mutex;

    std::unordered_map<glm::ivec3, ChunkMesh> meshes;
};
