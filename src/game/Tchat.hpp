#pragma once

#include <vector>
#include <string>

class Client;

struct TextMessage {
    std::string author;
    std::string message;
};

class Tchat
{
public:
    void sendTextMessage(Client& client, std::string str);
    std::vector<TextMessage> getTextMessages() const;

private:
    std::vector<TextMessage> _tchat;
};
