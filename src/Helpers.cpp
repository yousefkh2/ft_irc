#include "../include/CommandHandler.hpp"
#include "../include/Channel.hpp"
#include "../include/Utils.hpp"
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sys/socket.h>

void CommandHandler::handlePing(Client& client, const std::vector<std::string>& params) {
	std::string response;
	if (params.empty()) {
		response = ":" + std::string(SERVER_HOSTNAME) + " PONG " + std::string(SERVER_HOSTNAME);
	} else {
		response = ":" + std::string(SERVER_HOSTNAME) + " PONG " + std::string(SERVER_HOSTNAME) + " :" + params[0];
	}
	send(client.getFd(), (response + "\r\n").c_str(), response.length() + 2, 0);
}

// Helper function to send message to a client
void CommandHandler::sendToClient(Client& client, const std::string& message) {
    std::string fullMessage = ":" + std::string(SERVER_HOSTNAME) + " " + message + "\r\n";
    send(client.getFd(), fullMessage.c_str(), fullMessage.length(), 0);
}

// Helper function to validate channel names
bool CommandHandler::isValidChannelName(const std::string& name) {
    if (name.empty()) return false;
    
    // IRC channels typically start with # or &
    char first = name[0];
    return (first == '#' || first == '&') && name.length() > 1;
}

// Helper function to send message to all clients in a channel
void CommandHandler::sendToChannel(Channel* channel, const std::string& message, Client* exclude) {
    std::string fullMessage = message + "\r\n";
    
    for (Client* client : channel->getClients()) {
        if (client != exclude) {  // Don't send to excluded client
            send(client->getFd(), fullMessage.c_str(), fullMessage.length(), 0);
        }
    }
}
