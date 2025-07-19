#include "../include/CommandHandler.hpp"
#include "../include/Server.hpp"
#include "../include/Utils.hpp"

void CommandHandler::handlePing(Client& client, const std::vector<std::string>& params) {
	if (!params.empty()) {
		std::string response = "PONG : " + std::string(SERVER_HOSTNAME) + " :" + params[0];
		sendToClient(client, response);
		std::cout << "PING-PONGED\n"; 
	}
}

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
	const std::string& newNickname = params[0];
	if (newNickname.empty() || newNickname.length() > MAX_NICK_LEN) {
		sendNumeric(client, 432, newNickname + " :Erroneous nickname"); //ERR_ERRONEUSNICKNAME
		return ;
	}
	if (_server->isNicknameInUse(newNickname)) {
		sendNumeric(client, 433, newNickname + " :Nickname is already in use"); //ERR_NICKNAMEINUSE
		return ;
	}
	client.setNickname(newNickname);
	client.setNickSet(true);
	std::cout << "Client " << client.getFd()
		  << " set NICK to " << newNickname << "\n";
}