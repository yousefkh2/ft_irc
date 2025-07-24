#include "../include/CommandHandler.hpp"
#include "../include/Channel.hpp"
#include "../include/Server.hpp"
#include "../include/Utils.hpp"
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sys/socket.h>

void CommandHandler::handlePing(Client& client, const std::vector<std::string>& params) {
	std::string response;
	if (params.empty()) {
		response = ":" + std::string(SERVER_HOSTNAME) + " PONG " + std::string(SERVER_HOSTNAME);
	} else {
		response = ":" + std::string(SERVER_HOSTNAME) + " PONG " + std::string(SERVER_HOSTNAME) + " :" + params[0];
	}
	send(client.getFd(), (response + "\r\n").c_str(), response.length() + 2, 0);
}

// Helper function to send message to a client
void CommandHandler::sendToClient(Client& client, const std::string& message) {
    std::string fullMessage = ":" + std::string(SERVER_HOSTNAME) + " " + message + "\r\n";
    send(client.getFd(), fullMessage.c_str(), fullMessage.length(), 0);
}

// Helper function to validate channel names
bool CommandHandler::isValidChannelName(const std::string& name) {
    if (name.empty()) return false;
    
    // IRC channels typically start with # or &
    char first = name[0];
    return (first == '#' || first == '&') && name.length() > 1;
}

// Helper function to send message to all clients in a channel
void CommandHandler::sendToChannel(Channel* channel, const std::string& message, Client* exclude) {
    std::string fullMessage = message + "\r\n";
    
    for (Client* client : channel->getClients()) {
        if (client != exclude) {  // Don't send to excluded client
            send(client->getFd(), fullMessage.c_str(), fullMessage.length(), 0);
        }
    }
}

// Helper for JOIN command on how to join single channel
void CommandHandler::joinSingleChannel(Client &client, const std::string &channelName, const std::string &channelKey)
{
	if (!isValidChannelName(channelName))
	{
		sendNumeric(client, 403, channelName + " :No such channel");
		return;
	}
	Channel *channel = _server->getChannel(channelName);
	if (!channel)
	{
		channel = _server->createChannel(channelName);
	}
	if (!channel)
	{
		sendNumeric(client, 403, channelName + " :Channel creation failed");
		return;
	}
	if (channel->hasClient(&client))
	{
		return;
	}
	if (channel->hasKey())
	{
		if (channelKey.empty() || channelKey != channel->getKey())
		{
			sendNumeric(client, 475, channelName + " :Cannot join channel, password protected");
			return;
		}
	}
	if (channel->hasUserLimit())
	{
		if (channel->getClientCount() >= channel->getUserLimit())
		{
			sendNumeric(client, 471, channelName + " :Cannot join channel due to limit restriction");
			return;
		}
	}
	if (channel->isInviteOnly() && !channel->isInvited(&client))
	{
		sendNumeric(client, 473, channelName + " :Cannot join channel due to invite only mode");
		return;
	}
	channel->addClient(&client);
	client.joinChannel(channelName);
	if (channel->isInvited(&client))
	{
		channel->removeInvitedClient(&client);
	}
	std::string nick = client.nickname();
	std::string user = client.username();
	std::string joinMsg = ":" + nick + "!" + user + "@" + client.hostname() + " JOIN " + channelName;
	sendToChannel(channel, joinMsg);
	if (!channel->getTopic().empty())
	{
		sendToClient(client, "332 " + nick + " " + channelName + " :" + channel->getTopic());
	}
	else
	{
		sendToClient(client, "331 " + nick + " " + channelName + " :No topic is set");	
	}
	std::string namesList = "353 " + nick + " = " + channelName + " :";
	for (Client *c : channel->getClients())
	{
		if (c && !c->nickname().empty())
		{
			if (channel->isOperator(c))
			{
				namesList += "@";
			}
			namesList += c->nickname() + " ";
		}
	}
	// sendToChannel(channel, namesList);
	sendToClient(client, namesList);
	sendToClient(client, "366 " + nick + " " + channelName + " :End of /NAMES list");
	std::cout << "Client " << nick << " joined channel " + channelName << std::endl;
}
