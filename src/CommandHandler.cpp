#include "../include/CommandHandler.hpp"
#include <iostream>
#include <vector>

CommandHandler::CommandHandler(const std::string& password)
  : _password(password) {}

  void CommandHandler::handle(Client& client, const Command& cmd) {
	std::cout << "Handler processing: " << cmd.name << std::endl;
	if (cmd.name == "PASS") {
		handlePass(client, cmd.params);
	}
	else if (cmd.name == "NICK") {
		handleNick(client, cmd.params);
	}
	else if (cmd.name == "USER") {
		handleUser(client, cmd.params);
	}

	if (client.isRegistered()) {
		std::cout << "Client " << client.getFd()
				<< " fully registered (PASS, NICK, USER done)\n";
		// later: send welcome numerics 001, 002...
	}}

	void CommandHandler::handlePass(Client& client, const std::vector<std::string>& params) {
		if (params.size() < 1) {
			std::cerr << "PASS: missing parameter\n";
			return;
		}
		if (params[0] == _password) {
			client.setPassed(true);
			std::cout << "Client " << client.getFd() << " bad PASS\n";
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
  