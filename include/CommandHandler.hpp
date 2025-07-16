#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Client.hpp"
#include "Command.hpp"
#include "Server.hpp"

class Server;
class Channel;

using Params = std::vector<std::string>;

class CommandHandler {
	using CmdFn = void (CommandHandler::*)(Client&, const Params&);
	public:
	CommandHandler(const std::string& password, Server* server);

	/** 
	* process one parsed command for a given client.
	* if the client completes all THREE, mark as registered
	*/
	void handle(Client& client, const Command& cmd);

	private:
	void handlePass(Client& client, const std::vector<std::string>& params);
	void handleNick(Client& client, const std::vector<std::string>& params);
	void handleUser(Client& client, const std::vector<std::string>& params);
	void handleJoin(Client& client, const std::vector<std::string>& params);


	// Utility functions
    void sendToClient(Client& client, const std::string& message);
    void sendToChannel(Channel* channel, const std::string& message, Client* exclude = nullptr);
    bool isValidChannelName(const std::string& name);
	std::string _password;
	Server* _server;
	static const std::unordered_map<std::string, CmdFn> _dispatch_table;
};