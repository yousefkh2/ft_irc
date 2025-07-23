#include "../include/CommandHandler.hpp"
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
    
    // Just send the "End of WHO list" - no actual WHO data
    sendNumeric(client, 315, target + " :End of WHO list");
}
