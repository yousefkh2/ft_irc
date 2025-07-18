#include "../include/CommandHandler.hpp"
#include "../include/Server.hpp"

void CommandHandler::handleMode(Client& client, const std::vector<std::string>& params) {
    if (!client.isRegistered()) {
        sendNumeric(client, 451, ":You have not registered");
        return ;
    }
    if (params.empty()) {
        sendNumeric(client, 461, "MODE :Not enough parameters");
        return;
    }
    std::string target = params[0];
    if (isValidChannelName(target)) {
        handleChannelMode(client, params);
    } else {
        sendNumeric(client, 502, ":Cannot change mode for other users");
    }
}

void CommandHandler::handleChannelMode(Client& client, const std::vector<std::string>& params) {
    
}