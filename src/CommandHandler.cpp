#include "../include/CommandHandler.hpp"
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>



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
	{"USER", &CommandHandler::handleUser},
	{"JOIN", &CommandHandler::handleJoin}
};

CommandHandler::CommandHandler(const std::string& password, Server* server)
    : _password(password), _server(server) {}

  void CommandHandler::handle(Client& client, const Command& cmd) {
	std::cout << "Handler processing: " << cmd.name << std::endl;

		auto it = _dispatch_table.find(toUpperIrc(cmd.name));
		if (it != _dispatch_table.end()) {
			(this->*it->second)(client, cmd.params);
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

  void CommandHandler::handleJoin(Client& client, const std::vector<std::string>& params)
  {
	if (!client.isRegistered())
	{
		sendToClient(client, "451 :You have not registered");
		return;
	}
  }