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
    std::string channelName = params[0];
    Channel* channel = _server->getChannel(channelName);
    if (!channel) {
        sendNumeric(client, 403, channelName + " :No such channel");
        return;
    }
    if (!channel->hasClient(&client)) {
        sendNumeric(client, 442, channelName + " :You're not on that channel");
        return;
    }
    if (params.size() == 1) { // Shoz current mode if nothing provided
        std::string modeString = "+";
        std::string modeParams = "";
        if (channel->isInviteOnly())
            modeString += "i";
        if (channel->hasKey()) {
            modeString += "k";
            modeParams += " " + channel->getKey();
        }
        if (channel->hasUserLimit()) {
            modeString += "l";
            modeParams += " " + std::to_string(channel->getUserLimit());
        }
        if (modeString == "+")
            modeString = "+"; // Nothing set
        sendNumeric(client, 324, channelName + " " + modeString + modeParams);
        return ;
    }
    std::string modeString = params[1];
    size_t paramIndex = 2;
    bool adding = true;
    for (size_t i = 0; i < modeString.length(); ++i) {
        char mode = modeString[i];
        if (mode == '+') {
            adding = true;
            continue ;
        } else if (mode == '-') {
            adding = false;
            continue ;
        }
        if (!channel->isOperator(&client)) {
            sendNumeric(client, 482, channelName + " :You're not channel operator");
            return ;
        }
        switch (mode) {
            case 'i':
                handleInviteOnlyMode(client, channel, adding);
        }
    }
}
