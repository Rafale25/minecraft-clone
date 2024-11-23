#pragma once

#include "glad/gl.h"

#include "Context.hpp"
#include "Program.h"
#include "Shadowmap.hpp"

#include "Geometry.hpp"
#include "BufferAllocator.hpp"

class Camera;

class WorldRenderer
{
public:
    WorldRenderer(Context &context);

    void setDefaultRenderState();

    void render(const Camera &camera);

    void renderTerrain(const Program& program, const Camera& camera, bool use_frustum_culling=true);
    void renderTerrainDepth(const Camera &camera);

    void renderEntities(const Camera &camera);
    void renderEntitiesDepth(const Camera &camera);

    void renderSkybox(const Camera &camera);
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

public:
    GLuint ssbo_texture_handles;

    Program cube_shader{"./assets/shaders/cube.vs", "./assets/shaders/cube.fs"};
    Program cube_shadowmapping_shader{"./assets/shaders/cube_shadowmap.vs", "./assets/shaders/cube_shadowmap.fs"};
    Program mesh_shader{"./assets/shaders/mesh.vs", "./assets/shaders/mesh.fs"};
    Program skybox_shader{"./assets/shaders/skybox.vs", "./assets/shaders/skybox.fs"};

    Mesh skybox_quad = Geometry::quad_2d();

    const uint32_t MAX_COMMANDS = 5'000;
    BufferAllocator buffer_allocator_vertices{"BufferAllocatorVertice", 20'000 * sizeof(int), MAX_COMMANDS};
    BufferAllocator buffer_allocator_indices{"BufferAllocatorIndices", 20'000 * sizeof(int), MAX_COMMANDS};

    GLuint chunk_vao;
    GLuint draw_command_buffer;
    GLuint ssbo_chunk_positions;

};
