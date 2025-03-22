#include "Client.hpp"

#include <iostream>
#include <string>
#include <cstring>

#include "GameState.hpp"
#include "ByteBuffer.hpp"
#include "endianess.h"

void Client::decode(PacketId id, ByteBuffer buffer) {
    packets.at(id).decode(buffer);
}

void Client::init(std::vector<std::string>& tchat, const char* ip, int32_t port)
{
    _tchat = &tchat;

    _client.init();
    _client.connectToServer(ip, port);

    sendClientMetadataPacket(GameState::getRenderDistance(), "Rafale25");
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
    uint8_t buffer[35000] = {};
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
