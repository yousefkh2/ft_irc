#include "../include/Channel.hpp"
#include "../include/CommandHandler.hpp"
#include "../include/Server.hpp"

void CommandHandler::handleMode(Client& client, const std::vector<std::string>& params) {
    if (!client.isRegistered()) {
        sendNumeric(client, 451, ":You have not registered");
        return;
    }
    if (params.empty()) {
        sendNumeric(client, 461, "MODE :Not enough parameters");
        return;
    }
    std::string target = params[0];
    if (isValidChannelName(target)) {
        handleChannelMode(client, params);
    } else {
        // Handle user modes
        if (target == client.nickname()) {
            if (params.size() == 1) {
                // Query user modes
                sendNumeric(client, 221, "+");
            } else {
                sendNumeric(client, 221, "+");
            }
            return;
        }
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
    if (params.size() == 1) {
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
            modeString = "+";
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
                break ;
            case 't':
                handleTopicRestrictionMode(client, channel, adding);
                break ;
            case 'o':
                if (paramIndex < params.size()) {
                    handleOperatorMode(client, channel, adding, params[paramIndex]);
                    paramIndex++;
                } else {
                    sendNumeric(client, 461, "MODE :Not enough parameters");
                    return ;
                }
                break ;
            case 'k':
                if (adding) {
                    if (paramIndex < params.size()) {
                        handleChannelKeyMode(client, channel, true, params[paramIndex]);
                        paramIndex++;
                    } else {
                        sendNumeric(client, 461, "MODE :not enough parameters");
                        return ;
                    }
                } else {
                    handleChannelKeyMode(client, channel, false, "");
                }
                break ;
            case 'l':
                if (adding) {
                    if (paramIndex < params.size()) {
                        handleUserLimitMode(client, channel, true, params[paramIndex]);
                        paramIndex++;
                    } else {
                        sendNumeric(client, 461, "MODE :Not enough parameters");
                        return ;
                    }
                } else {
                    handleUserLimitMode(client, channel, false, "");
                }
                break ;
            case 'b':
                sendNumeric(client, 368, channelName + " :End of channel ban list");
                break;
            default:
                sendNumeric(client, 472, channelName + " :Unknown mode char");
                break;
        }
    }
}

void CommandHandler::handleInviteOnlyMode(Client& client, Channel* channel, bool adding) {
    if (adding != channel->isInviteOnly()) {
        channel->setInviteOnly(adding);
        std::string modeMsg = ":" + client.nickname() + "!" + client.username() + "@localhost MODE " + channel->getName() + " " + (adding ? "+i" : "-i");
        sendToChannel(channel, modeMsg);
        std::cout << "Channel " << channel->getName() << " invite-only mode " << (adding ? "enabled" : "disabled") << " by " << client.nickname() << std::endl;
    }
}

void CommandHandler::handleTopicRestrictionMode(Client& client, Channel* channel, bool adding) {
    if (adding != channel->hasTopicRestriction()) {
        channel->setTopicRestriction(adding);
        std::string modeMsg = ":" + client.nickname() + "!" + client.username() + "@localhost MODE " + channel->getName() + " " + (adding ? "+t" : "-t");
        sendToChannel(channel, modeMsg);
        std::cout << "Channel " << channel->getName() << " topic restriction " << (adding ? "enabled" : "disabled") << " by " << client.nickname() << std::endl;
    }
}

void CommandHandler::handleOperatorMode(Client& client, Channel* channel, bool adding, const std::string& targetNick) {
    Client* targetClient = nullptr;
    for (Client* c : channel->getClients()) {
        if (c && c->nickname() == targetNick) {
            targetClient = c;
            break ;
        }
    }
    if (!targetClient) {
        sendNumeric(client, 401, targetNick + " :No such cik/channel");
        return ;
    }
    if (adding) {
        if (!channel->isOperator(targetClient)) {
            channel->addOperator(targetClient);
            std::string modeMsg = ":" + client.nickname() + "!" + client.username() + "@localhost MODE " + channel->getName() + " +o " + targetNick;
            sendToChannel(channel, modeMsg);
            std::cout << "Client " << targetNick << " given operator privileges in " << channel->getName() << " by " << client.nickname() << std::endl;
        }
    } else {
        if (channel->isOperator(targetClient)) { //Canot remove yourself from OP
            if (targetClient == &client) {
                size_t opCount = 0;
                for (Client* c : channel->getClients()) {
                    if (channel->isOperator(c)) {
                        opCount++;
                    }
                }
                if (opCount == 1) {
                    sendNumeric(client, 482, channel->getName() + " :Cannot remove yourself as the only operator");
                    return ;
                }
            }
            channel->removeOperator(targetClient);
            std::string modeMsg = ":" + client.nickname() + "!" + client.username() + "@localhost MODE " +  channel->getName() + " -o " + targetNick;
            sendToChannel(channel, modeMsg);
            std::cout << "Client " << targetNick << " removed operator privileges in " << channel->getName() << " by " << client.nickname() << std::endl;
        }
    }
}

void CommandHandler::handleChannelKeyMode(Client& client, Channel* channel, bool adding, const std::string& key) {
    if (adding) {
        if (key.empty()) {
            sendNumeric(client, 461, "MODE :Not enough parameters");
            return ;
        }
        if (key.find(' ') != std::string::npos || key.length() > 50) { //Validate Key
            sendNumeric(client, 525, channel->getName() + " :Invalid channel key");
            return;
        }
        channel->setKey(key);
        std::string modeMsg = ":" + client.nickname() + "!" + client.username() + "@localhost MODE " + channel->getName() + " +k";
        sendToChannel(channel, modeMsg);
        std::cout << "Channel " << channel->getName() << " key set by " << client.nickname() << std::endl;
    } else {
        if (channel->hasKey()) {
            channel->removeKey();
            std::string modeMsg = ":" + client.nickname() + "!" + client.username() + "@localhost MODE " + channel->getName() + " -k";
            sendToChannel(channel, modeMsg);
            std::cout << "Channel " << channel->getName() << " key removed by " << client.nickname() << std::endl;
        }
    }
}

void CommandHandler::handleUserLimitMode(Client& client, Channel* channel, bool adding, const std::string& limitStr) {
    if (adding) {
        if (limitStr.empty()) {
            sendNumeric(client, 461, "MODE :Not enough parameters");
            return ;
        }
        size_t limit;
        try {
            limit = std::stoul(limitStr);
        } catch (const std::exception&) {
            sendNumeric(client, 525, channel->getName() + " :Invalid user limit");
            return ;
        }
        if (limit == 0 || limit > 1000) {
            sendNumeric(client, 525, channel->getName() + " :Invalid user limit");
            return ;
        }
        channel->setUserLimit(limit);
        std::string modeMsg = ":" + client.nickname() + "!" + client.username() + "@localhost MODE " + channel->getName() + " +l " + std::to_string(limit);
        sendToChannel(channel, modeMsg);
        std::cout << "Channel " << channel->getName() << " user limit set to " << limit << " by " << client.nickname() << std::endl;
    } else {
        if (channel->hasUserLimit()) {
            channel->removeUserLimit();
            std::string modeMsg = ":" + client.nickname() + "!" + client.username() + "@localhost MODE " + channel->getName() + " -l";
            sendToChannel(channel, modeMsg);
            std::cout << "Channel " << channel->getName() << " user limit removed by " << client.nickname() << std::endl;
        }
    } 
}