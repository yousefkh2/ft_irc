#include "../include/CommandHandler.hpp"
#include "../include/Client.hpp"
#include "../include/Server.hpp"

void CommandHandler::handleJoin(Client &client,
  const std::vector<std::string> &params)
{
  if (!client.isRegistered()) {
    sendNumeric(client, 451, ":You have not registered");
    return;
  }
  if (params.empty()) {
    sendNumeric(client, 461, "JOIN :Not enough parameters");
    return;
  }
  std::cout << "JOIN parameters: ";
  for (size_t i = 0; i < params.size(); ++i) {
    std::cout << "[" << i << "]=\"" << params[i] << "\" ";
  }
  std::cout << std::endl;
  std::vector<std::string> channelNames;
  std::vector<std::string> channelKeys;
  std::string channelsParam = params[0];
  size_t keyStartIndex = 1;
  for (size_t i = 1; i < params.size(); ++i) {
    if (!params[i].empty() && (params[i][0] == '#' || params[i-1].back() == ',')) {
      if (!channelsParam.empty() && channelsParam.back() != ',') {
        channelsParam += ",";
      }
      channelsParam += params[i];
      keyStartIndex = i + 1;
    } else {
      break;
    }
  }
  std::cout << "Reconstructed channels param: \"" << channelsParam << "\"" << std::endl;
  size_t start = 0;
  size_t end = channelsParam.find(',');
  while (end != std::string::npos) {
    std::string channelName = channelsParam.substr(start, end - start);
    channelName.erase(0, channelName.find_first_not_of(" \t"));
    channelName.erase(channelName.find_last_not_of(" \t") + 1);
    if (!channelName.empty()) {
      channelNames.push_back(channelName);
      std::cout << "Added channel: \"" << channelName << "\"" << std::endl;
    }
    start = end + 1;
    end = channelsParam.find(',', start);
  }
  std::string lastChannel = channelsParam.substr(start);
  lastChannel.erase(0, lastChannel.find_first_not_of(" \t"));
  lastChannel.erase(lastChannel.find_last_not_of(" \t") + 1);
  if (!lastChannel.empty()) {
    channelNames.push_back(lastChannel);
    std::cout << "Added last channel: \"" << lastChannel << "\"" << std::endl;
  }
  if (keyStartIndex < params.size()) {
    std::string keysParam = params[keyStartIndex];
    for (size_t i = keyStartIndex + 1; i < params.size(); ++i) {
      keysParam += "," + params[i];
    }
    size_t start = 0;
    size_t end = keysParam.find(',');
    while (end != std::string::npos) {
      std::string key = keysParam.substr(start, end - start);
      key.erase(0, key.find_first_not_of(" \t"));
      key.erase(key.find_last_not_of(" \t") + 1);
      channelKeys.push_back(key);
      start = end + 1;
      end = keysParam.find(',', start);
    }
    std::string lastKey = keysParam.substr(start);
    lastKey.erase(0, lastKey.find_first_not_of(" \t"));
    lastKey.erase(lastKey.find_last_not_of(" \t") + 1);
    channelKeys.push_back(lastKey);
  }
  std::cout << "Total channels to join: " << channelNames.size() << std::endl;
  for (size_t i = 0; i < channelNames.size(); ++i) {
    std::string currentChannel = channelNames[i];
    std::string currentKey = (i < channelKeys.size()) ? channelKeys[i] : "";
    std::cout << "Attempting to join channel: \"" << currentChannel << "\"" << std::endl;
    joinSingleChannel(client, currentChannel, currentKey);
  }
}

void CommandHandler::handlePart(Client &client, const std::vector<std::string> &params)
{

	if (!client.isRegistered())
	{
		sendNumeric(client, 451, ":You have not registered");
		return;
	}
	if (params.empty())
	{
		sendNumeric(client, 461, "PART :Not enough parameters");
		return;
	}
	std::string channelName = params[0];
	std::string partMessage = params.size() > 1 ? params[1] : "";
	if (!isValidChannelName(channelName))
	{
		sendNumeric(client, 403, channelName + " :Channel does not exist");
		return;
	}
	Channel *channel = _server->getChannel(channelName);
	if (!channel)
	{
		sendNumeric(client, 403, channelName + " :Channel does not exist");
		return;
	}
	if (!channel->hasClient(&client))
	{
		sendNumeric(client, 442, channelName + " :You're not on that channel");
		return;
	}

	std::string nick = client.nickname();
	std::string user = client.username();
	std::string partMsg = ":" + nick + "!" + user + "@" + client.hostname() + " PART " + channelName;
	if (!partMessage.empty())
		partMsg += " :" + partMessage;
	sendToChannel(channel, partMsg);

	bool wasOperator = channel->isOperator(&client);
	channel->removeClient(&client);
	client.leaveChannel(channelName);

	if (wasOperator && channel->getClientCount() > 0)
	{
		Client *newOp = *channel->getClients().begin();
		channel->addOperator(newOp);
		std::string modeMsg = ":server MODE " + channelName + " +o " + newOp->nickname();
		sendToChannel(channel, modeMsg);
		std::string noticeMsg = ":server NOTICE " + channelName + " :" + newOp->nickname() + " has been promoted to operator";
		sendToChannel(channel, noticeMsg);
		std::cout << "Client " << newOp->nickname() << " promoted to operator in " << channelName << std::endl;
	}

	if (channel->getClientCount() == 0)
	{
		_server->removeChannel(channelName);
	}

	std::cout << "Client " << nick << " left channel " << channelName << std::endl;
}

void CommandHandler::handleTopic(Client &client, const std::vector<std::string> &params)
{
	if (!client.isRegistered())
	{
		sendNumeric(client, 451, ":You have not registered");
		return;
	}
	if (params.empty())
	{
		sendNumeric(client, 461, "TOPIC :Not enough parameters");
		return;
	}
	std::string channelName = params[0];
	if (!isValidChannelName(channelName))
	{
		sendNumeric(client, 403, channelName + " :No such channel");
		return;
	}
	Channel *channel = _server->getChannel(channelName);
	if (!channel)
	{
		sendNumeric(client, 403, channelName + " :Channel does not exist");
		return;
	}
	if (!channel->hasClient(&client))
	{
		sendNumeric(client, 442, channelName + " :You're not on that channel");
		return;
	}
	std::string nick = client.nickname();
	if (params.size() == 1)
	{
		if (channel->getTopic().empty())
		{
			sendNumeric(client, 331, channelName + " :No topic is set on this channel");
		}
		else
		{
			sendNumeric(client, 332, channelName + " :" + channel->getTopic());
		}
		return;
	}
	std::string newTopic;
	for (size_t i = 1; i < params.size(); ++i)
	{
		if (i > 1)
			newTopic += " ";
		newTopic += params[i];
	}
	if (channel->hasTopicRestriction() && !channel->isOperator(&client))
	{
		sendNumeric(client, 482, channelName + " :You're not channel operator");
		return;
	}
	channel->setTopic(newTopic);
	std::string user = client.username();
	std::string topicMsg = ":" + nick + "!" + user + "@" + client.hostname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
	sendToChannel(channel, topicMsg);
	if (channel->getTopic().empty())
	{
		sendToClient(client, ":server 331 " + nick + " " + channelName + " :No topic is set");
	}
	else
	{
		sendToClient(client, ":server 332 " + nick + " " + channelName + " :" + channel->getTopic());
	}
	std::cout << "Topic for " << channelName << " changed by " << nick << " to: " << newTopic << std::endl;
}

void CommandHandler::handleKick(Client &client, const std::vector<std::string> &params)
{
	if (!client.isRegistered())
	{
		sendNumeric(client, 451, ":You have not registered");
		return;
	}
	if (params.size() < 2)
	{ // Usage: KICK #channel nickname :reason
		sendNumeric(client, 461, "KICK :Not enough parameters");
		return;
	}
	std::string channelName = params[0];
	std::string targetNick = params[1];
	std::string kickReason = "No reason given";
	if (params.size() > 2)
	{
		kickReason = params[2];
		if (kickReason[0] == ':')
		{
			kickReason = kickReason.substr(1);
		}
		for (size_t i = 3; i < params.size(); ++i)
		{
			kickReason += " " + params[i];
		}
	}
	if (!isValidChannelName(channelName))
	{
		sendNumeric(client, 403, channelName + " :No such channel");
		return;
	}
	Channel *channel = _server->getChannel(channelName);
	if (!channel)
	{
		sendNumeric(client, 403, channelName + " :No such channel");
		return;
	}
	if (!channel->hasClient(&client))
	{
		sendNumeric(client, 442, channelName + " :You're not on that channel");
		return;
	}
	if (!channel->isOperator(&client))
	{ // the moderator is the only one allowed to kick someone
		sendNumeric(client, 482, channelName + " :You're not channel operator");
		return;
	}
	Client *targetClient = nullptr; // target user to kick
	for (Client *c : channel->getClients())
	{
		if (c && c->nickname() == targetNick)
		{
			targetClient = c;
			break;
		}
	}
	if (!targetClient)
	{ // target must be present in the channel
		sendNumeric(client, 401, targetNick + " :No such nick/channel");
		return;
	}
	if (targetClient == &client)
	{ // moderator cannot kick himself
		sendNumeric(client, 484, channelName + " :Cannot kick yourself");
		return;
	}
	std::string kickMsg = ":" + client.nickname() + "!" + client.username() + "@" + client.hostname() + " KICK " + channelName + " " + targetNick + " :" + kickReason;
	if (channel->isOperator(targetClient))
	{
		channel->removeOperator(targetClient);
	}
	if (channel->isInvited(targetClient))
	{
		channel->removeInvitedClient(targetClient);
	}
	sendToChannel(channel, kickMsg);
	channel->removeClient(targetClient);
	targetClient->leaveChannel(channelName);
	std::cout << "Client " << client.nickname() << " kicked " << targetNick
						<< " from " << channelName << std::endl;
}

void CommandHandler::handleInvite(Client &client, const std::vector<std::string> &params)
{
	if (!client.isRegistered())
	{
		sendNumeric(client, 451, ":You have not registered");
		return;
	}
	if (params.size() < 2)
	{ // usage INVITE <nickname> <channel>
		sendNumeric(client, 461, "INVITE :Not enough parameters");
		return;
	}
	std::string targetNick = params[0];
	std::string channelName = params[1];
	if (!isValidChannelName(channelName))
	{
		sendNumeric(client, 403, channelName + " :No such channel");
		return;
	}
	Channel *channel = _server->getChannel(channelName);
	if (!channel)
	{
		sendNumeric(client, 403, channelName + " :No such channel");
		return;
	}
	if (!channel->hasClient(&client))
	{
		sendNumeric(client, 442, channelName + " :You're not on that channel");
		return;
	}
	if (!channel->isOperator(&client))
	{
		sendNumeric(client, 482, channelName + " :You're not channel operator");
		return;
	}
	Client *targetClient = nullptr;
	for (auto &clientPair : _server->getClients())
	{
		if (clientPair.second.nickname() == targetNick)
		{
			targetClient = const_cast<Client *>(&clientPair.second);
			break;
		}
	}
	if (!targetClient)
	{
		sendNumeric(client, 401, targetNick + " :No such nick or channel");
		return;
	}
	if (channel->hasClient(targetClient))
	{ // Cannot invite people that are already in the channel
		sendNumeric(client, 443, targetNick + " " + channelName + " :is already on channel");
		return;
	}
	channel->addInvitedClient(targetClient);
	sendNumeric(client, 341, targetNick + " " + channelName);
	std::string inviteMsg = ":" + client.nickname() + "!" + client.username() + " @" + client.hostname() + " INVITE " + targetNick + " " + channelName;
	sendToClient(*targetClient, inviteMsg);
	std::cout << "Client " << client.nickname() << " invited " << targetNick << " to channel " << channelName << std::endl;
}
