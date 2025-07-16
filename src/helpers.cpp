#include "../include/CommandHandler.hpp"
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

// Helper function to send message to a client
void CommandHandler::sendToClient(Client& client, const std::string& message) {
    std::string fullMessage = message + "\r\n";
    send(client.getFd(), fullMessage.c_str(), fullMessage.length(), 0);
}

