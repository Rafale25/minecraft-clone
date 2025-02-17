#include "Client.hpp"

#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>

#include "endianess.h"
#include "command_line_args.h"
#include "World.hpp"
#include "byte_manipulation.h"
#include "ByteBuffer.h"

void Client::decode(PacketId id, ByteBuffer buffer) {
    packets.at(id).decode(buffer);
}

void Client::init(std::vector<std::string>& tchat, const char* ip)
{
    _tchat = &tchat;

    _client.init();
    _client.connectToServer(ip, DEFAULT_PORT);

    int32_t render_distance = 8;
    if (global_argc > 2)
        render_distance = std::atoi(global_argv[2]);
    sendClientMetadataPacket(render_distance, "Rafale25");
}

void Client::Start()
{
    _stop_thread = false;
    client_thread = std::thread(&Client::clientThreadFunc, this);
}

void Client::Stop()
{
    _stop_thread = true;
    _client.closeConnection();
    client_thread.join();
}

void Client::clientThreadFunc()
{
    uint8_t buffer[5000] = {};
    int32_t recv_size = -1;

    while (!_stop_thread)
    {
        _client.waitForData(_stop_thread); // wait for data to read

        recv_size = _client.receiveAll(buffer, 1);
        if (recv_size == -1) {
            std::cout << "recv failed: return -1" << std::endl;
            break;
        }

        const PacketId id = (PacketId)buffer[0];

        if (packets.find(id) == packets.end()) {
            printf("Invalid Packet id %d\n", id);
            return;
        }

        const size_t packet_size = packets.at(id).size;
        recv_size = _client.receiveAll(buffer, packet_size);
        if (recv_size == -1) {
            std::cout << "recv failed: return -1" << std::endl;
            break;
        }

        decode(id, ByteBuffer(buffer, packet_size, ByteBuffer::ByteOrder::BE));
    }
}


void Client::sendPacket(const void *buf, size_t size)
{
    _client.sendD(buf, size);
}
