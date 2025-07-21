#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Client.hpp"
#include "Command.hpp"
#include <iostream>


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
	void sendToClient(Client& client, const std::string& message);

	private:
	void handlePing(Client& client, const std::vector<std::string>& params);
	void handleCap(Client& client, const std::vector<std::string>& params);
	void handlePass(Client& client, const std::vector<std::string>& params);
	void handleNick(Client& client, const std::vector<std::string>& params);
	void handleUser(Client& client, const std::vector<std::string>& params);
	void sendWelcomeSequence(Client& client);
	void sendNumeric(Client& client, int code, const std::string& message);
	void handleJoin(Client& client, const std::vector<std::string>& params);
	void handlePart(Client& client, const std::vector<std::string>& params);
	void handleTopic(Client& client, const std::vector<std::string>& params);
	void handleKick(Client& client, const std::vector<std::string>& params);
	void handleInvite(Client& client, const std::vector<std::string>& params);
	void handleMode(Client& client, const std::vector<std::string>& params);
	void handlePrivmsg(Client& client, const std::vector<std::string>& params);

	// Utility functions
    void sendToChannel(Channel* channel, const std::string& message, Client* exclude = nullptr);
    bool isValidChannelName(const std::string& name);

	// Mode utilities
	void handleChannelMode(Client& client, const std::vector<std::string>& params);
	void handleInviteOnlyMode(Client& client, Channel* channel, bool adding);
	void handleTopicRestrictionMode(Client& client, Channel* channel, bool adding);
	void handleOperatorMode(Client& client, Channel* channel, bool adding, const std::string& targetNick);
    void handleChannelKeyMode(Client& client, Channel* channel, bool adding, const std::string& key);
    void handleUserLimitMode(Client& client, Channel* channel, bool adding, const std::string& limitStr);

	std::string _password;
	Server* _server;
	static const std::unordered_map<std::string, CmdFn> _dispatch_table;
};