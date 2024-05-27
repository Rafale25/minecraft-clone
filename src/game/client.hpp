#pragma once

#include <unordered_map>
#include <thread>
#include <mutex>
#include <deque>

#include "glm/gtx/hash.hpp"

class TextureManager;
class World;
struct Chunk;

class Client
{
public:
    Client() {};
    Client(World& world, TextureManager& texture_manager, const char* ip);

    void Start();
    void client_thread_func();

public:
    std::deque<std::function<void()>> task_queue;

    // TODO: have list of new chunks with VAO not yet computed
    // NOTE: new chunk need to be assigned to the chunks list only when the VAO is computed
    // std::vector<Chunk> new_chunks;


    std::mutex task_queue_mutex;
    int client_id = -1;
    int client_socket;

private:
    std::thread client_thread;
    World *world;

    TextureManager* texture_manager;
};
