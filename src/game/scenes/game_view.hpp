#pragma once

#include <unordered_map>
#include <math.h>
#include <mutex>

// #define GLM_SWIZZLE
#include "glm/gtx/hash.hpp"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
// #include <glm/gtx/vec_swizzle.hpp>

#include "FastNoiseLite.h"

#include "view.hpp"
#include "camera.hpp"
#include "program.h"

#include "texture_manager.hpp"
#include "chunk.hpp"
#include "client.hpp"
#include "entity.hpp"
#include "world.hpp"

#include "imgui.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <endian.h>


Chunk generateChunk(glm::ivec3 pos, FastNoiseLite &noise, TextureManager &texture_manager)
{
    Chunk chunk;
    chunk.pos = pos;

    glm::vec3 chunkPosWorld = pos * 16;

    for (int z = 0 ; z < 16 ; ++z) {
    for (int x = 0 ; x < 16 ; ++x) {
        int height = 1.0f + (noise.GetNoise(chunkPosWorld.x + x, chunkPosWorld.z + z) * 0.5f + 0.5f) * 15.0f;

        for (int y = 0 ; y < 16 ; ++y) {
            int index = z * 16*16 + y * 16 + x;

            float world_y = chunkPosWorld.y + y;

            if (world_y == height)
                chunk.blocks[index] = BlockType::Grass;
            else if (world_y < height)
                chunk.blocks[index] = BlockType::Dirt;
            else
                chunk.blocks[index] = BlockType::Air;

            // chunk.blocks[index] = BlockType::Grass;
            // chunk.blocks[index] = chunkPosWorld.y + y < height ? BlockType::Grass : BlockType::Air;
            // chunk.blocks[index] = rand() % 3 == 0 ? BlockType::Grass : BlockType::Air;
        }
    }
    }

    chunk.computeChunckVAO(texture_manager);

    return chunk;
}

static void print_buf(const char *title, const unsigned char *buf, size_t buf_len)
{
    size_t i = 0;
    fprintf(stdout, "%s [ ", title);
    for(i = 0; i < buf_len; ++i)
    fprintf(stdout, "%02X%s", buf[i], ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
    std::cout << ']' <<  std::endl;
}

std::tuple<BlockType, glm::ivec3, glm::vec3> BlockRaycast(World& world, glm::vec3 origin, glm::vec3 direction, int maxSteps=16)
{
    glm::vec3 rayPos = origin;
    glm::vec3 mapPos = glm::ivec3(glm::floor(rayPos));

    glm::vec3 deltaDist = abs(glm::vec3(glm::length(direction)) / direction);
	glm::vec3 rayStep = sign(direction);
	glm::vec3 sideDist = (sign(direction) * (mapPos - rayPos) + (sign(direction) * 0.5f) + 0.5f) * deltaDist;

	glm::vec3 mask;
    glm::vec3 normal;

	for (int i = 0; i < maxSteps; i++) {
        auto block = world.get_block(mapPos);
        normal = -mask * rayStep;

        if (block != BlockType::Air) {
            // printf("mapPos: %f %f %f\n", mapPos.x, mapPos.y, mapPos.z);
            return std::tuple<BlockType, glm::ivec3, glm::vec3>({block, mapPos, normal});
        };

		mask = glm::step(sideDist, glm::vec3(sideDist.y, sideDist.z, sideDist.x)) * glm::step(sideDist, glm::vec3(sideDist.z, sideDist.x, sideDist.y));
		sideDist += mask * deltaDist;
		mapPos += mask * rayStep;

        // printf("mapPos: %f %f %f\n", mapPos.x, mapPos.y, mapPos.z);
	}

    return std::tuple<BlockType, glm::ivec3, glm::vec3>({BlockType::Air, mapPos, normal});
}

class GameView: public View {
    public:
        GameView(Context& ctx): View(ctx)
        {
            glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            int width, height;
            glfwGetWindowSize(ctx.window, &width, &height);

            // camera = new OrbitCamera(
            //     glm::vec3(0.0f), M_PI/4, M_PI/4, 50.0f,
            //     60.0f, (float)width / (float)height, 0.1f, 1000.0f
            // );
            camera = FPSCamera{
                glm::vec3(0.0f, 30.0, 0.0f), 0.0f, 0.0f,
                60.0f, (float)width / (float)height, 0.01f, 1000.0f
            };

            texture_manager.loadAllTextures();

            cube_shader = new Program("./assets/shaders/cube.vs", "./assets/shaders/cube.fs");
            mesh_shader = new Program("./assets/shaders/mesh.vs", "./assets/shaders/mesh.fs");

            client.Start();

            #if 0
                FastNoiseLite noise;
                noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

                int SIZE_X = 8;
                int SIZE_Y = 1;
                int SIZE_Z = 8;
                for (int x = -SIZE_X ; x < SIZE_X ; ++x) {
                for (int y = -SIZE_Y ; y < SIZE_Y ; ++y) {
                for (int z = -SIZE_Z ; z < SIZE_Z ; ++z) {
                    Chunk c = generateChunk({x, y, z}, noise, texture_manager);
                    world.chunks[c.pos] = c;
                }
                }
                }
            #endif
        }

        void onUpdate(double time_since_start, float dt)
        {
            float dx = ctx.keyState[GLFW_KEY_A] - ctx.keyState[GLFW_KEY_D];
            float dy = ctx.keyState[GLFW_KEY_LEFT_CONTROL] - ctx.keyState[GLFW_KEY_SPACE];
            float dz = ctx.keyState[GLFW_KEY_W] - ctx.keyState[GLFW_KEY_S];

            camera.move(glm::vec3(dx, dy, dz));
            camera.update(dt);

            auto [blocktype, world_pos, normal] = BlockRaycast(world, camera.getPosition(), camera.forward);
            raycastBlocktype = blocktype;
            raycastWorldPos = world_pos;
            raycastNormal = normal;
            // printf("Blocktype: %d, worldpos: %d %d %d, normal: %f %f %f\n", (int)blocktype, world_pos.x, world_pos.y, world_pos.z, normal.x, normal.y, normal.z);

            for (auto& [key, chunk] : world.chunks)
            {
                if (chunk.vao_initialized == false) {
                    world.chunks_mutex.lock();
                    chunk.computeChunckVAO(texture_manager);
                    world.chunks_mutex.unlock();
                }
            }

            for (auto& entity : world.entities)
            {
                entity.smooth_transform.position = glm::mix(entity.smooth_transform.position, entity.transform.position, 0.075f);
                entity.smooth_transform.rotation = glm::mix(entity.smooth_transform.rotation, entity.transform.rotation, 0.075f);
            }

            // client task_queue //
            client.task_queue_mutex.lock();
            for (auto &task: client.task_queue) {
                task();
            }
            client.task_queue.clear();
            client.task_queue_mutex.unlock();

            network_timer -= dt;
            if (network_timer <= 0.0f) {
                network_timer = 1.0f / 20.0f;
                networkUpdate();
            }
        }

        void networkUpdate()
        {
            if (client.client_id == -1) return;

            struct __attribute__ ((packed)) moveEntityPacket {
                uint8_t id;
                int entityId;
                int x, y, z, yaw, pitch; // float encoded in int
            } packet;

            glm::vec3 pos = camera.getPosition();
            float yaw = camera.getYaw();
            float pitch = camera.getPitch();

            packet.id = 0x00; // update entity //
            packet.entityId = htobe32(client.client_id);
            packet.x = htobe32(*(uint32_t*)&pos.x);
            packet.y = htobe32(*(uint32_t*)&pos.y);
            packet.z = htobe32(*(uint32_t*)&pos.z);
            packet.yaw = htobe32(*(uint32_t*)&yaw);
            packet.pitch = htobe32(*(uint32_t*)&pitch);

            send(client.client_socket, &packet, sizeof(packet), 0);
        }

        void onDraw(double time_since_start, float dt)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CW);

            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cube_shader->use();
            cube_shader->setMat4("u_projectionMatrix", camera.getProjection());
            cube_shader->setMat4("u_viewMatrix", camera.getView());
            cube_shader->setVec3("u_view_position", camera.getPosition());

            world.chunks_mutex.lock();
            for (const auto& [key, chunk] : world.chunks)
            {
                cube_shader->setVec3("u_chunkPos", chunk.pos * 16);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, chunk.ssbo_texture_handles);
                glBindVertexArray(chunk.VAO);
                glDrawArrays(GL_TRIANGLES, 0, chunk.vertices_count);
            }
            world.chunks_mutex.unlock();

            mesh_shader->use();
            mesh_shader->setMat4("u_projectionMatrix", camera.getProjection());
            mesh_shader->setMat4("u_viewMatrix", camera.getView());

            for (auto& entity : world.entities)
            {
                mesh_shader->setMat4("u_modelMatrix", entity.smooth_transform.getMatrix());
                entity.draw();
            }

            gui(dt);
        }

        void gui(float dt)
        {
            ctx.imguiNewFrame();
            ImGui::Begin("Debug");

            ImGui::Text("%.4f ms", dt);
            ImGui::Text("%.2f fps", 1.0f / dt);

            glm::vec3 camera_pos = camera.getPosition();
            ImGui::Text("center: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
            ImGui::Text("forward: %.2f, %.2f, %.2f", camera.forward.x, camera.forward.y, camera.forward.z);
            // ImGui::Text("yaw: %.2f", camera.getYaw());
            ImGui::Text("block in hand: %d", (int)blockInHand);
            // ImGui::Text("pitch: %.2f", camera.getPitch());

            ImGui::End();
            ctx.imguiRender();
        }

        void breakBlock(glm::ivec3 world_pos)
        {
            // Send server block update
            struct __attribute__ ((packed)) moveEntityPacket {
                uint8_t id;
                uint8_t blockType;
                int x, y, z;
            } packet;

            packet.id = 0x01; // update block //
            packet.blockType = 0;
            packet.x = htobe32(*(uint32_t*)&world_pos.x);
            packet.y = htobe32(*(uint32_t*)&world_pos.y);
            packet.z = htobe32(*(uint32_t*)&world_pos.z);

            send(client.client_socket, &packet, sizeof(packet), 0);
        }

        void placeBlock(glm::ivec3 world_pos, BlockType blocktype)
        {
            // Send server block update
            struct __attribute__ ((packed)) moveEntityPacket {
                uint8_t id;
                uint8_t blockType;
                int x, y, z;
            } packet;

            packet.id = 0x01; // update block //
            packet.blockType = (int)blocktype;
            packet.x = htobe32(*(uint32_t*)&world_pos.x);
            packet.y = htobe32(*(uint32_t*)&world_pos.y);
            packet.z = htobe32(*(uint32_t*)&world_pos.z);

            send(client.client_socket, &packet, sizeof(packet), 0);
        }

        void onKeyPress(int key)
        {
            if (key == GLFW_KEY_C) {
                _cursorEnable = !_cursorEnable;

                if (_cursorEnable)
                    glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                else
                    glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }

        void onKeyRelease(int key)
        {
        }

        void onMousePress(int x, int y, int button) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                breakBlock(raycastWorldPos);
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                placeBlock(raycastWorldPos + glm::ivec3(raycastNormal), blockInHand);
            }
        }

        void onMouseDrag(int x, int y, int dx, int dy)
        {
            if (ImGui::GetIO().WantCaptureMouse) return;

            // camera.setYaw( camera.getYaw() - (dx * 0.005f) );
            // camera.setPitch( camera.getPitch() + (dy * 0.005f) );
        }

        void onMouseScroll(int scroll_x, int scroll_y)
        {
            // float scale = 1.0f;
            // if (ctx.keyState[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS)
            //     scale = 8.0f;
            // camera.setDistance( camera.getDistance() - (scroll_y * scale) );

            int block = ((int)blockInHand + scroll_y) % ((int)BlockType::LAST-1);
            if (block < 1)
                block += (int)BlockType::LAST-1;
            blockInHand = (BlockType)block;
        }

        void onMouseMotion(int x, int y, int dx, int dy)
        {
            if (!_cursorEnable)
                camera.onMouseMotion(x, y, dx, dy);
        }

        void onResize(int width, int height)
        {
            glViewport(0, 0, width, height);
        }

    private:
        // OrbitCamera* camera;
        FPSCamera camera;

        Program* cube_shader;
        Program* mesh_shader;

        World world;
        Client client{world};

        float network_timer = 1.0f;

        TextureManager texture_manager;

        int _cursorEnable = false;

        // std::deque<>

        BlockType blockInHand = BlockType::Grass;

        BlockType raycastBlocktype;
        glm::vec3 raycastNormal;
        glm::ivec3 raycastWorldPos;
};
