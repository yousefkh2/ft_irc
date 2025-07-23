#include "../include/CommandHandler.hpp"
#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include "../include/Client.hpp"

void CommandHandler::handleWho(Client& client, const std::vector<std::string>& params)
{
    if (!client.isRegistered()) {
        sendNumeric(client, 451, ":You have not registered");
        return;
    }
    std::string target = "*";
    if (!params.empty()) {
        target = params[0];
    }
    std::string nick = client.nickname();
    if (target[0] == '#') {
        Channel* channel = _server->getChannel(target);
        if (!channel) {
            sendNumeric(client, 403, target + " :No such channel");
            sendNumeric(client, 315, target + " :End of WHO list");
            return;
        }
        if (!channel->hasClient(&client)) {
            sendNumeric(client, 315, target + " :End of WHO list");
            return;
        }
        for (Client* c : channel->getClients()) {
            if (c && !c->nickname().empty()) {
                std::string flags = "";
                if (channel->isOperator(c)) {
                    flags += "@";
                }

                std::string whoReply = "352 " + nick + " " + target + " " + 
                                     c->username() + " " + c->hostname() + " server " + 
                                     c->nickname() + " " + flags + " :0 " + c->nickname();
                sendToClient(client, whoReply);
            }
        }
    }
    else if (target == "*") {
        std::set<Client*> visibleClients;
        visibleClients.insert(&client);

        for (Client* c : visibleClients) {
            std::string flags = "";
            
            std::string whoReply = "352 " + nick + " * " + 
                                 c->username() + " " + c->hostname() + " server " + 
                                 c->nickname() + " " + flags + " :0 " + c->nickname();
            sendToClient(client, whoReply);
        }
    }
    else {
        Client* targetClient = nullptr;
        for (const auto& pair : _server->getClients()) {
            if (pair.second.nickname() == target) {
                targetClient = const_cast<Client*>(&pair.second);
                break;
            }
        }
        if (targetClient) {
            bool canSee = true;

            if (canSee || targetClient == &client) {
                std::string flags = "";
                
                std::string whoReply = "352 " + nick + " * " + 
                                     targetClient->username() + " " + targetClient->hostname() + " server " + 
                                     targetClient->nickname() + " " + flags + " :0 " + targetClient->nickname();
                sendToClient(client, whoReply);
            }
        }
    }
    sendNumeric(client, 315, target + " :End of WHO list");
    std::cout << "WHO command executed by " << nick << " for target: " << target << std::endl;
}
