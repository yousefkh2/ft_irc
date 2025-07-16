#include "../include/CommandHandler.hpp"
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sys/socket.h>
#include <unistd.h>

using Params = std::vector<std::string>;
using CmdFn = void (CommandHandler::*)(Client&, const Params&);

std::string toUpperIrc(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

const std::unordered_map<std::string, CmdFn> CommandHandler::_dispatch_table = {
	{"PASS", &CommandHandler::handlePass},
	{"NICK", &CommandHandler::handleNick},
	{"USER", &CommandHandler::handleUser},
	{"PING", &CommandHandler::handlePing},
	{"PONG", &CommandHandler::handlePong},
	{"QUIT", &CommandHandler::handleQuit}
};

CommandHandler::CommandHandler(const std::string& password)
  : _password(password) {}

void CommandHandler::handle(Client& client, const Command& cmd) {
	std::cout << "Handler processing: " << cmd.name << std::endl;

	auto it = _dispatch_table.find(toUpperIrc(cmd.name));
	if (it != _dispatch_table.end()) {
		(this->*it->second)(client, cmd.params);
	} else {
		// Send "command not found" error for registered clients
		if (client.isRegistered()) {
			std::string error = ":localhost 421 " + client.nickname() + " " + cmd.name + " :Unknown command\r\n";
			sendToClient(client, error);
		}
	}

	// Check if client just became registered
	if (client.isRegistered()) {
		std::cout << "Client " << client.getFd()
				<< " fully registered (PASS, NICK, USER done)\n";
		sendWelcomeMessages(client);
	}
}

void CommandHandler::sendToClient(Client& client, const std::string& message) {
	send(client.getFd(), message.c_str(), message.length(), 0);
}

void CommandHandler::sendWelcomeMessages(Client& client) {
	// Only send welcome messages once
	if (client.wasWelcomed()) {
		return;
	}
	client.setWelcomed(true);

	std::string nick = client.nickname();
	
	// RFC 1459 welcome messages
	std::string welcome = ":localhost 001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n";
	std::string yourhost = ":localhost 002 " + nick + " :Your host is localhost, running version ircserv-1.0\r\n";
	std::string created = ":localhost 003 " + nick + " :This server was created today\r\n";
	std::string myinfo = ":localhost 004 " + nick + " localhost ircserv-1.0 o o\r\n";
	
	sendToClient(client, welcome);
	sendToClient(client, yourhost);
	sendToClient(client, created);
	sendToClient(client, myinfo);
}

void CommandHandler::handlePass(Client& client, const std::vector<std::string>& params) {
	if (client.isRegistered()) {
		std::string error = ":localhost 462 " + client.nickname() + " :You may not reregister\r\n";
		sendToClient(client, error);
		return;
	}
	
	if (params.size() < 1) {
		std::string error = ":localhost 461 * PASS :Not enough parameters\r\n";
		sendToClient(client, error);
		return;
	}
	
	if (params[0] == _password) {
		client.setPassed(true);
		std::cout << "Client " << client.getFd() << " GOOD PASS\n";
	} else {
		std::cout << "Client " << client.getFd() << " BAD PASS\n";
		std::string error = ":localhost 464 * :Password incorrect\r\n";
		sendToClient(client, error);
		// In a real server, you might want to close the connection here
	}
}

void CommandHandler::handleNick(Client& client, const std::vector<std::string>& params) {
	if (params.empty()) {
		std::string error = ":localhost 431 * :No nickname given\r\n";
		sendToClient(client, error);
		return;
	}
	
	std::string new_nick = params[0];
	
	// Basic nickname validation (you can expand this)
	if (new_nick.length() > 9) {
		std::string error = ":localhost 432 * " + new_nick + " :Erroneous nickname\r\n";
		sendToClient(client, error);
		return;
	}
	
	// TODO: Check if nickname is already in use by another client
	
	std::string old_nick = client.nickname();
	client.setNickname(new_nick);
	client.setNickSet(true);
	
	std::cout << "Client " << client.getFd() << " set NICK to " << new_nick << "\n";
	
	// If client is already registered, send nick change confirmation
	if (client.isRegistered() && !old_nick.empty()) {
		std::string nick_change = ":" + old_nick + " NICK :" + new_nick + "\r\n";
		sendToClient(client, nick_change);
	}
}

void CommandHandler::handleUser(Client& client, const std::vector<std::string>& params) {
	if (client.isRegistered()) {
		std::string error = ":localhost 462 " + client.nickname() + " :You may not reregister\r\n";
		sendToClient(client, error);
		return;
	}
	
	if (params.size() < 4) {
		std::string error = ":localhost 461 * USER :Not enough parameters\r\n";
		sendToClient(client, error);
		return;
	}
	
	client.setUsername(params[0]);
	client.setUserSet(true);
	std::cout << "Client " << client.getFd() << " set USER to " << params[0] << "\n";
}

void CommandHandler::handlePing(Client& client, const std::vector<std::string>& params) {
	std::string pong = ":localhost PONG localhost";
	if (!params.empty()) {
		pong += " :" + params[0];
	}
	pong += "\r\n";
	sendToClient(client, pong);
}

void CommandHandler::handlePong(Client& client, const std::vector<std::string>& params) {
	// Client responded to our PING - just log it
	std::cout << "Client " << client.getFd() << " sent PONG\n";
}

void CommandHandler::handleQuit(Client& client, const std::vector<std::string>& params) {
	std::string quit_msg = "Quit";
	if (!params.empty()) {
		quit_msg = params[0];
	}
	
	std::cout << "Client " << client.getFd() << " quit: " << quit_msg << "\n";
	
	// Send quit confirmation
	std::string quit_response = ":" + client.nickname() + " QUIT :" + quit_msg + "\r\n";
	sendToClient(client, quit_response);
	
	// Close the connection
	close(client.getFd());
}
