#include "../include/CommandHandler.hpp"
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sys/socket.h>


using Params = std::vector<std::string>;
using CmdFn = void (CommandHandler::*)(Client&, const Params&);


std::string toUpperIrc(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

/*
better i think
for (char& c : result)
    c = std::toupper(c);
*/

const std::unordered_map<std::string, CmdFn> CommandHandler::_dispatch_table = {
	{"PASS", &CommandHandler::handlePass},
	{"NICK", &CommandHandler::handleNick},
	{"USER", &CommandHandler::handleUser}
};

CommandHandler::CommandHandler(const std::string& password)
  : _password(password) {}

  void CommandHandler::sendNumeric(Client& client, int code, const std::string& message) {
	std::string response = ":ft_irc.server " + std::to_string(code) + " " + 
	client.nickname() + " " + message + "\r\n";
	send(client.getFd(), response.c_str(), response.length(), 0);
}

void CommandHandler::sendWelcomeSequence(Client& client) {
	std::string nick = client.nickname();
	std::string user = client.username();

	// 001 RPL_WELCOME
	sendNumeric(client, 1, ":Welcome to the IRC Network " + nick + "!" + user + "@localhost");
	// 002 RPL_YOURHOST
	sendNumeric(client, 2, ":Your host is ft_irc.server, running version 1.0");
	// 003 RPL_CREATED
	sendNumeric(client, 3, ":This server was created today");
	// 004 RPL_MYINFO
	sendNumeric(client, 4, "ft_irc.server 1.0 - -");
}

  void CommandHandler::handle(Client& client, const Command& cmd) {
	std::cout << "Handler processing: " << cmd.name << std::endl;

		bool wasRegistered = client.isRegistered();

		auto it = _dispatch_table.find(toUpperIrc(cmd.name));
		if (it != _dispatch_table.end()) {
			(this->*it->second)(client, cmd.params);
		}

	if (!wasRegistered && client.isRegistered()) {
		std::cout << "Client " << client.getFd()
				<< " fully registered (PASS, NICK, USER done)\n";
		sendWelcomeSequence(client);
	}}

	



	void CommandHandler::handlePass(Client& client, const std::vector<std::string>& params) {
		if (params.size() < 1) {
			std::cerr << "PASS: missing parameter\n";
			return;
		}
		if (params[0] == _password) {
			client.setPassed(true);
			std::cout << "Client " << client.getFd() << " GOOD PASS\n";
		} else {
			std::cout << "Client " << client.getFd() << " BAD PASS\n";
			// later: send ERR_PASSWDMISTMATCH and close
		}
	}

	void CommandHandler::handleNick(Client& client, const std::vector<std::string>& params) {
		if (params.empty()) {
			std::cerr << "NICK: missing parameter\n";
			return;
		}
		client.setNickname(params[0]);
		client.setNickSet(true);
		std::cout << "Client " << client.getFd()
              << " set NICK to " << params[0] << "\n";
	}

	void CommandHandler::handleUser(Client& client,  const std::vector<std::string>& params) {
		if (params.size() < 4) {
			std::cerr << "USER: missing parameters\n";
			return;
		}
		client.setUsername(params[0]);
		client.setUserSet(true);
		std::cout << "Client " << client.getFd()
              << " set USER to " << params[0] << "\n";
	}
  