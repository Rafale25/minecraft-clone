#include "Client.hpp"
#include "GameState.hpp"
#include "ByteBuffer.hpp"
#include "Logger.hpp"
#include <string>
#include <cstring>

void Client::decode(PacketId id, ByteBuffer buffer) {
    packets.at(id).decode(buffer);
}

void Client::init(std::vector<std::string>& tchat, const char* ip, int32_t port)
{
    m_tchat = &tchat;

    if (m_client.init() == 0) { // success
        logI("Connection initialized successfully");
    }

    if (m_client.connectToServer(ip, port) == 0) {
        logI("Connected to server {}:{}", ip, port);
    }

    sendClientMetadataPacket(GameState::getRenderDistance(), "Rafale25");
}

void Client::Start()
{
    m_stopThread = false;
    m_clientThread = std::thread(&Client::clientThreadFunc, this);
}

void Client::Stop()
{
    m_stopThread = true;
    m_client.closeConnection();
    m_clientThread.join();
}

void Client::clientThreadFunc()
{
    uint8_t buffer[35000] = {};
    int32_t recv_size = -1;

    while (!m_stopThread)
    {
        m_client.waitForData(m_stopThread); // wait for data to read

        recv_size = m_client.receiveAll(buffer, 1);
        if (recv_size == -1) {
            logE("Failed to receive from server (packet id)");
            break;
        }

        const PacketId id = (PacketId)buffer[0];

        if (packets.find(id) == packets.end()) {
            logE("Invalid Packet id {}", (int)id);
            return;
        }

        const size_t packet_size = packets.at(id).size;
        recv_size = m_client.receiveAll(buffer, packet_size);
        if (recv_size == -1) {
            logE("Failed to receive from server (packet data)");
            break;
        }

        decode(id, ByteBuffer(buffer, packet_size, ByteBuffer::ByteOrder::BE));
    }
}


void Client::sendPacket(const void *buf, size_t size)
{
    m_client.sendD(buf, size);
}
