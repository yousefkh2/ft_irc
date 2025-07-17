#include "../include/CommandHandler.hpp"



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

	// TODO: Check if nickname is already in use by another client
	// sendNumeric(client, 433, params[0] + " :Nickname is already in use");
	client.setNickname(params[0]);
	client.setNickSet(true);
	std::cout << "Client " << client.getFd()
		  << " set NICK to " << params[0] << "\n";
}