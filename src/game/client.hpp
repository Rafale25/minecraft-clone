#pragma once

#include <unordered_map>
#include <thread>
#include <mutex>

#include "glm/gtx/hash.hpp"

class TextureManager;
struct Chunk;

class Client
{
public:
    Client() {};
    Client(std::unordered_map<glm::ivec3, Chunk> *chunks, std::mutex &chunks_mutex);

    void Start();
    void client_thread_func();

private:
    int client_socket;
    int client_id = -1;

    std::thread client_thread;

    std::unordered_map<glm::ivec3, Chunk> *chunks;
    std::mutex *chunks_mutex;
};
