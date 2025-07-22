#include "../include/CommandHandler.hpp"
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "../include/Server.hpp"
#include "../include/Utils.hpp"
#include <sys/socket.h>
#include <sstream>


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
	{"PING", &CommandHandler::handlePing},
	{"CAP", &CommandHandler::handleCap},
	{"PASS", &CommandHandler::handlePass},
	{"NICK", &CommandHandler::handleNick},
	{"USER", &CommandHandler::handleUser},
	{"JOIN", &CommandHandler::handleJoin},
	{"PART", &CommandHandler::handlePart},
	{"TOPIC", &CommandHandler::handleTopic},
	{"KICK", &CommandHandler::handleKick},
	{"INVITE", &CommandHandler::handleInvite},
	{"MODE", &CommandHandler::handleMode},
	{"PRIVMSG", &CommandHandler::handlePrivmsg}
};

CommandHandler::CommandHandler(const std::string& password, Server* server)
    : _password(password), _server(server) {}

void CommandHandler::sendNumeric(Client& client, int code, const std::string& message) {
	//without this will send 1 instead of 001
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(3) << code;
	std::string numericCode = oss.str();

	std::string response = ":" + std::string(SERVER_HOSTNAME) + " " + numericCode + " " + client.nickname() + " " + message + "\r\n";
	send(client.getFd(), response.c_str(), response.length(), 0);

	std::cout << "got numeric " << code << std::endl;
}

void CommandHandler::sendWelcomeSequence(Client& client) {
	std::string nick = client.nickname();
	std::string user = client.username();

	// 001 RPL_WELCOME
	sendNumeric(client, 1, ":Welcome to the IRC Network " + nick + "!" + user + "@" + client.hostname());
	// 002 RPL_YOURHOST
	sendNumeric(client, 2, ":Your host is " + std::string(SERVER_HOSTNAME) + ", running version 1.0");
	// 003 RPL_CREATED
	sendNumeric(client, 3, ":This server was created today");
	// 004 RPL_MYINFO
	sendNumeric(client, 4, std::string(SERVER_HOSTNAME) + " 1.0 - -");
}

  void CommandHandler::handle(Client& client, const Command& cmd) {
		if (cmd.name != "PING") {
        std::cout << "Handler processing: " << cmd.name << std::endl;
    		}
		bool wasRegistered = client.isRegistered();
		std::string upperCmd = toUpperIrc(cmd.name);

		if (!client.isRegistered() && 
				upperCmd != "CAP" && 
				upperCmd != "PASS" && 
				upperCmd != "NICK" && 
				upperCmd != "USER") {
			sendNumeric(client, 451, ":You have not registered");
			return;
		}

		auto it = _dispatch_table.find(upperCmd);
		if (it != _dispatch_table.end()) {
			(this->*it->second)(client, cmd.params);
		} else {
			sendNumeric(client, 421, cmd.name + " :Unknown command"); //ERR_UNKNOWNCOMMAND
		}

		if (!wasRegistered && client.isRegistered()) {
			std::cout << "Client " << client.getFd()
					<< " fully registered (PASS, NICK, USER done)\n";
			sendWelcomeSequence(client);
			sendNumeric(client, 376, ":End of /MOTD command");
		}
	}



void CommandHandler::sendRaw(Client& client, const std::string& line)
{
    send(client.getFd(), line.c_str(), line.size(), 0);
}