#pragma once

#include "glad/gl.h"

#include "Context.hpp"
#include "Program.h"
#include "Shadowmap.hpp"

#include "Geometry.hpp"
#include "BufferAllocator.hpp"

#include "ChunkMesh.hpp"
#include "glm/gtx/hash.hpp"

class Camera;

class WorldRenderer
{
public:
    WorldRenderer(Context &context);

    void render(const Camera &camera);

    void signalDeletedChunk(const glm::ivec3& chunk_pos);
    void onResize(int width, int height);
    // void signalAddedChunk(const glm::ivec3& chunk_pos);

    // ChunkMesh makeChunkMesh(const ChunkRawMesh& raw_mesh);

private:
    void setDefaultRenderState();

    void renderTerrain(const Program& program, const Camera& camera, bool use_frustum_culling=true);
    void renderTerrainDepth(const Camera &camera);

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

    Program cube_shader{"./assets/shaders/cube.vs", "./assets/shaders/cube.fs"};
    Program cube_shadowmapping_shader{"./assets/shaders/cube_shadowmap.vs", "./assets/shaders/cube_shadowmap.fs"};
    Program mesh_shader{"./assets/shaders/mesh.vs", "./assets/shaders/mesh.fs"};
    Program postprocessing_shader{"./assets/shaders/postprocess.vs", "./assets/shaders/postprocess.fs"};

    Mesh skybox_quad = Geometry::quad_2d();

    const float chunk_view_distance = 16.0f * 16.0f;
    const float chunk_delete_offset = 16.0f * 16.0f;

    const uint32_t MAX_COMMANDS = 20'000;
    BufferAllocator buffer_allocator_vertices{"BufferAllocatorVertice", 25'000 * sizeof(int), MAX_COMMANDS};
    BufferAllocator buffer_allocator_indices{"BufferAllocatorIndices", 25'000 * sizeof(int), MAX_COMMANDS};

    GLuint chunk_vao;
    GLuint draw_command_buffer;
    GLuint ssbo_chunk_positions;

    std::unordered_map<glm::ivec3, ChunkMesh> meshes;
};
