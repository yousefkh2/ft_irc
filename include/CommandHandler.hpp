#pragma once
#include <string>
#include "Client.hpp"
#include "Command.hpp"

class CommandHandler {
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

	std::string _password;
};