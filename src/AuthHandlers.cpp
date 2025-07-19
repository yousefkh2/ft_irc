#include "../include/CommandHandler.hpp"
#include "../include/Server.hpp"



void CommandHandler::handlePass(Client& client, const std::vector<std::string>& params) {
	if (client.isRegistered()) {
		sendNumeric(client, 462, ":You may not reregister"); //ERR_ALREADYREGISTERED
		return;
	}
	if (params.size() < 1) {
		sendNumeric(client, 461, "PASS :Not enough parameters");  // ERR_NEEDMOREPARAMS
		return;
	}
	if (params[0] != _password) {
		sendNumeric(client, 464, ":Password incorrect");  // ERR_PASSWDMISMATCH
		return;
	}
	client.setPassed(true);
}



void CommandHandler::handleUser(Client& client,  const std::vector<std::string>& params) {
	if (params.size() < 4) {
		sendNumeric(client, 461, "USER :Not enough parameters");
		return;
	}
	client.setUsername(params[0]);
	client.setUserSet(true);
	std::cout << "Client " << client.getFd()
		  << " set USER to " << params[0] << "\n";
}

void CommandHandler::handleNick(Client& client, const std::vector<std::string>& params) {
	if (params.empty()) {
		sendNumeric(client, 431, ":No nickname given"); //ERR_NONICKNAMEGIVEN
		return;
	}

	std::string requestedNick = params[0];
	
	for (const auto& clientPair : _server->getClients()) {
		const Client& otherClient = clientPair.second;
		if (&otherClient != &client && //not checking against self
			otherClient.hasNick() && 
			otherClient.nickname() == requestedNick) {
			sendNumeric(client, 433, requestedNick + " :Nickname is already in use");
			return;
		}
	}
	std::string oldNick = client.hasNick() ? client.nickname() : "";

	client.setNickname(requestedNick);
	client.setNickSet(true);

	if (client.isRegistered() && !oldNick.empty()) {
		std::string nickMsg = ":" + oldNick + "!" + client.username() + "@localhost NICK :" + requestedNick;
		_server->broadcastToClientChannels(&client, nickMsg, *this);
	}
	std::cout << "Client " << client.getFd()
		  << " set NICK to " << requestedNick << "\n";
}