#pragma once

#include <unordered_map>
#include <map>
#include <math.h>

#include "glm/gtx/hash.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "FastNoiseLite.h"

#include "view.hpp"
#include "camera.hpp"
#include "program.h"

#include "texture_manager.hpp"
#include "chunk.hpp"
#include "enums.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>


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

Chunk readChunk(uint8_t *buffer, TextureManager &texture_manager)
{
    uint8_t *head = &buffer[0];
    int x, y, z;

    // skip packet id
    head += sizeof(uint8_t);

    x = *(int*)&head[0];
    head += sizeof(int);

    y = *(int*)&head[0];
    head += sizeof(int);

    z = *(int*)&head[0];
    head += sizeof(int);

    Chunk chunk;
    chunk.pos = glm::ivec3(htonl(x), htonl(y), htonl(z)) / 16;

    glm::vec3 chunkPosWorld = chunk.pos * 16;

    for (int i = 0 ; i < 16*16*16 ; ++i) {
        uint8_t byte = *(uint8_t*)&head[0];
        head += sizeof(uint8_t);

        /* convert to BlackoutBurst indexing -_- */
        int x = i % 16;
        int y = (i / 16) % 16;
        int z = i / (16 * 16);

        int index = x * 16*16 + y * 16 + z;

        chunk.blocks[index] = (BlockType)byte;

        // switch (byte)
        // {
        //     case 0:
        //         chunk.blocks[i] = BlockType::Air;
        //         break;
        //     case 1:
        //         chunk.blocks[i] = BlockType::Grass;
        //         break;
        // }
    }

    chunk.computeChunckVAO(texture_manager);

    return chunk;
}

class GameView: public View {
    public:
        GameView(Context& ctx): View(ctx)
        {
            int width, height;
            glfwGetWindowSize(ctx.window, &width, &height);

            camera = new OrbitCamera(
                glm::vec3(0.0f), M_PI/4, M_PI/4, 20.0f,
                60.0f, (float)width / (float)height, 0.1f, 1000.0f
            );

            noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

            texture_manager.loadAllTextures();

            cube_shader = new Program("./assets/shaders/cube.vs", "./assets/shaders/cube.fs");


#if 0
            int SIZE_X = 4;
            int SIZE_Y = 1;
            int SIZE_Z = 4;
            for (int x = -SIZE_X ; x < SIZE_X ; ++x) {
            for (int y = -SIZE_Y ; y < SIZE_Y ; ++y) {
            for (int z = -SIZE_Z ; z < SIZE_Z ; ++z) {
                Chunk c = generateChunk({x, y, z}, noise, texture_manager);
                chunks[c.pos] = c;
            }
            }
            }
#else

            // -- Sockets -- //
            int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

            // specifying address
            sockaddr_in serverAddress;
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
            serverAddress.sin_port = htons(15000);

            // sending connection request
            connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

            // recieving data
            uint8_t buffer[5000] = { 0 };
            recv(clientSocket, buffer, sizeof(buffer), 0);
            printf("%d\n", buffer[0]);
            // print_buf("Buffer: ", buffer, 5000);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

            while (1)
            {
                int bytes_received = 0;
                bool should_break = false;

                while (1) {
                    int recv_size = recv(clientSocket, &buffer[bytes_received], 5000 - bytes_received, 0);
                    bytes_received += recv_size;
                    // printf("bytes_received: %d %d\n", recv_size, bytes_received);
                    if (bytes_received == 5000)
                        break;
                    if (recv_size == -1) {
                        should_break = true;
                        break;
                    }
                }
                if (should_break) break;

                if (buffer[0] != 0x05) {
                    continue;
                }

                Chunk c = readChunk(buffer, texture_manager);
                chunks[c.pos] = c;
            }

            printf("Chunks count: %ld\n", chunks.size());

            // close(clientSocket);
#endif
        }

        void onUpdate(float time_since_start, float dt)
        {

        }

        void onDraw(float time_since_start, float dt)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CW);

            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cube_shader->use();
            cube_shader->setMat4("u_projectionMatrix", camera->getProjection());
            cube_shader->setMat4("u_viewMatrix", camera->getView());

            for (const auto& [key, chunk] : chunks)
            {
                cube_shader->setVec3("u_chunkPos", chunk.pos * 16);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, chunk.ssbo_texture_handles);
                glBindVertexArray(chunk.VAO);
                glDrawArrays(GL_TRIANGLES, 0, chunk.vertices_count);
            }

            gui(dt);
        }

        void gui(float dt)
        {
            ctx.imguiNewFrame();
            ImGui::Begin("Debug");

            ImGui::Text("%.4f ms", dt);
            ImGui::Text("%.2f fps", 1.0f / dt);

            glm::vec3 camera_pos = camera->getPosition();
            ImGui::Text("center: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
            ImGui::Text("yaw: %.2f", camera->getYaw());
            ImGui::Text("pitch: %.2f", camera->getPitch());

            ImGui::End();
            ctx.imguiRender();
        }

        void onKeyPress(int key)
        {
        }

        void onKeyRelease(int key)
        {
        }

        void onMouseDrag(int x, int y, int dx, int dy)
        {
            if (ImGui::GetIO().WantCaptureMouse) return;

            camera->setYaw( camera->getYaw() - (dx * 0.005f) );
            camera->setPitch( camera->getPitch() + (dy * 0.005f) );
        }

        void onMouseScroll(int scroll_x, int scroll_y)
        {
            float scale = 1.0f;
            if (ctx.keyState[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS)
                scale = 8.0f;

            camera->setDistance( camera->getDistance() - (scroll_y * scale) );
        }

        void onResize(int width, int height)
        {
            glViewport(0, 0, width, height);
        }

    private:
        OrbitCamera* camera;
        Program* cube_shader;

        std::unordered_map<glm::ivec3, Chunk> chunks;

        FastNoiseLite noise;
        TextureManager texture_manager;

        // std::thread;
};
