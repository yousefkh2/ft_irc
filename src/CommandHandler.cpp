#include "../include/CommandHandler.hpp"
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <Server.hpp>



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
		if (!client.isRegistered()) // Check if client is registered
		{
			sendToClient(client, "451 :You have not registered");
			return ;
		}
		if (params.empty()) // Check if channel name is provided
		{
        	sendToClient(client, "461 JOIN :Not enough parameters");
        	return ;
    	}
		std::string channelName = params[0];
		if (!isValidChannelName(channelName)) // Validate channel name (must start with # or &)
		{
			sendToClient(client, "403 " + channelName + " :No such channel");
			return ;
		}
		Channel* channel = _server->getChannel(channelName); // Get or create channel
		if (!channel)
			channel = _server->createChannel(channelName);
		if (channel->hasClient(&client)) // Check if client is already in channel
			return ; // Silently ignore if already in channel
		channel->addClient(&client); // Add client to channel
		// Send JOIN message to all clients in channel (including the joiner)
		std::string joinMsg = ":" + client.nickname() + "!" + client.username() + "@localhost JOIN " + channelName;
    	sendToChannel(channel, joinMsg);
		if (!channel->getTopic().empty()) // Send topic if it exists
			sendToClient(client, "332 " + client.nickname() + " " + channelName + " :" + channel->getTopic());
		// Send names list (list of users in channel)
		std::string namesList = "353 " + client.nickname() + " = " + channelName + " :";
		for (Client* c : channel->getClients())
		{
			if (channel->isOperator(c))
				namesList += "@"; // @ prefix for operators
			namesList += c->nickname() + " ";
		}
		sendToClient(client, namesList);
		sendToClient(client, "366 " + client.nickname() + " " + channelName + " :End of /NAMES list");
		std::cout << "Client " << client.nickname() << " joined channel " << channelName << std::endl;
  	}