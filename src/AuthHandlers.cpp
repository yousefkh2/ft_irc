#include "../include/CommandHandler.hpp"
#include "../include/Server.hpp"
#include "../include/Server.hpp"
#include "../include/Utils.hpp"

// irssi will wait for CAP LS response, might as well recognise CAP
void CommandHandler::handleCap(Client& client, const std::vector<std::string>& params) {
	if (params.empty()) {
		return;
	}
	if (params[0] == "LS") {
		sendToClient(client, "CAP * LS :"); // no capabilities supported
	}
}

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
		_server->disconnectClient(client.getFd());
		return;

	}

	
	client.setPassed(true);
}

void CommandHandler::handleUser(Client& client, const std::vector<std::string>& params) {
	if (client.isRegistered()) {
		sendNumeric(client, 462, ":You may not reregister"); // ERR_ALREADYREGISTERED
		return;
	}

	if (params.size() < 4) {
		sendNumeric(client, 461, "USER :Not enough parameters");
		return;
	}

	if (!client.hasUser()) {
		client.setUsername(params[0]);
		client.setUserSet(true);
		std::cout << "Client " << client.getFd()
			  << " set USER to " << params[0] << "\n";
	}
}

void CommandHandler::handleNick(Client& client, const std::vector<std::string>& params) {
	if (params.empty()) {
		sendNumeric(client, 431, ":No nickname given"); //ERR_NONICKNAMEGIVEN
		return;
	}
	const std::string& requestedNick = params[0];
	if (requestedNick.empty() || requestedNick.length() > MAX_NICK_LEN || 
        requestedNick.find(' ') != std::string::npos || requestedNick[0] == '#') {
		sendNumeric(client, 432, requestedNick + " :Erroneous nickname"); //ERR_ERRONEUSNICKNAME
		return ;
	}
	if (_server->isNicknameInUse(requestedNick) &&
		(!client.hasNick() || client.nickname() != requestedNick)) {
		sendNumeric(client, 433, requestedNick + " :Nickname is already in use"); // ERR_NICKNAMEINUSE
		return;
	}

	std::string oldNick = client.hasNick() ? client.nickname() : "";

	client.setNickname(requestedNick);
	client.setNickSet(true);

	if (client.isRegistered() && !oldNick.empty() && oldNick != requestedNick) {
		std::string nickMsg = ":" + oldNick + "!" + client.username() + "@" + client.hostname() + " NICK :" + requestedNick + "\r\n";
		sendRaw(client, nickMsg);
		_server->broadcastToClientChannels(&client, nickMsg, *this);
	}
	std::cout << "Client " << client.getFd()
		  << " set NICK to " << requestedNick << "\n";
}
