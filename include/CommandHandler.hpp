#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Client.hpp"
#include "Command.hpp"

using Params = std::vector<std::string>;

class CommandHandler {
	using CmdFn = void (CommandHandler::*)(Client&, const Params&);
	public:
	CommandHandler(const std::string& password);

	/** 
	* process one parsed command for a given client.
	* if the client completes all THREE, mark as registered
	*/
	void handle(Client& client, const Command& cmd);

	private:
	void handlePass(Client& client, const std::vector<std::string>& params);
	void handleNick(Client& client, const std::vector<std::string>& params);
	void handleUser(Client& client, const std::vector<std::string>& params);
	void sendWelcomeSequence(Client& client);
	void sendNumeric(Client& client, int code, const std::string& message);


	std::string _password;
	static const std::unordered_map<std::string, CmdFn> _dispatch_table;
};