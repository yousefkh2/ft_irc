#include "../include/CommandHandler.hpp"
#include "../include/Server.hpp"

void CommandHandler::handleJoin(Client &client,
  const std::vector<std::string> &params) {
if (!client.isRegistered()) {
sendNumeric(client, 451, ":You have not registered");
return;
}

if (params.empty()) {
sendNumeric(client, 461, "JOIN :Not enough parameters");
return;
}

std::string channelName = params[0];
std::string channelKey = params.size() > 1 ? params[1] : "";
if (!isValidChannelName(channelName)) {
sendNumeric(client, 403, channelName + " :No such channel");
return;
}

Channel *channel = _server->getChannel(channelName);
if (!channel) {
channel = _server->createChannel(channelName);
}

// Add null pointer check after channel creation
if (!channel) {
sendNumeric(client, 403, channelName + " :Channel creation failed");
return;
}

if (channel->hasClient(&client)) {
return;
}

if (channel->hasKey()) {
  if (channelKey.empty() || channelKey != channel->getKey()) {
    sendNumeric(client, 475, channelName + " :Cannot join channel, password protected");
    return ;
  }
}

// Chek for user limit
if (channel->hasUserLimit()) {
  if (channel->getClientCount() >= channel->getUserLimit()) {
    sendNumeric(client, 471, channelName + " :Cannot join channel due to limit restriction");
    return ;
  }
}

// Check if channel is invite-only and user is not invited
if (channel->isInviteOnly() && !channel->isInvited(&client)) {
  sendNumeric(client, 473, channelName + " :Cannot join channel due to invite only mode");
  return ;
}

channel->addClient(&client);

// If user was invited, remove them from the invited list (they're now in the channel)
if (channel->isInvited(&client)) {
  channel->removeInvitedClient(&client);
}
std::string nick = client.nickname();
std::string user = client.username();

// Send JOIN message to all clients in channel (including the joiner)
std::string joinMsg = ":" + nick + "!" + user + "@localhost JOIN " + channelName;
sendToChannel(channel, joinMsg);

// Send topic if it exists
if (!channel->getTopic().empty()) {
sendToClient(client, "332 " + nick + " " + channelName + " :" + channel->getTopic());
}

// Send names list (list of users in channel)
std::string namesList = "353 " + nick + " = " + channelName + " :";
for (Client *c : channel->getClients()) {
// Add null pointer check for clients in channel
if (c && !c->nickname().empty()) {
if (channel->isOperator(c)) {
namesList += "@"; // @ prefix for operators
}
namesList += c->nickname() + " ";
}
}
sendToClient(client, namesList);
sendToClient(client, "366 " + nick + " " + channelName + " :End of /NAMES list");

std::cout << "Client " << nick << " joined channel " << channelName << std::endl;
}

void CommandHandler::handlePart(Client& client, const std::vector<std::string>& params)
{
	
    if (!client.isRegistered())
    {
      sendNumeric(client, 451, ":You have not registered");
      return ;
    }
    if (params.empty())
    {
      sendNumeric(client, 461, "PART :Not enough parameters");
      return ;
    }
    std::string channelName = params[0];
    std::string partMessage = params.size() > 1 ? params[1] : "";
    if (!isValidChannelName(channelName))
    {
      sendNumeric(client, 403, channelName + " :Channel does not exist");
      return ;
    }
    Channel* channel = _server->getChannel(channelName);
    if (!channel)
    {
      sendNumeric(client, 403, channelName + " :Channel does not exist");
      return ;
    }
    if (!channel->hasClient(&client))
    {
      sendNumeric(client, 442, channelName + " :You're not on that channel");
      return ;
    }

    std::string nick = client.nickname();
    std::string user = client.username();
    std::string partMsg = ":" + nick + "!" + user + "@localhost PART " + channelName;
    if (!partMessage.empty())
      partMsg += " :" + partMessage;
    sendToChannel(channel, partMsg);

    bool wasOperator = channel->isOperator(&client);
    channel->removeClient(&client);

	
    if (wasOperator && channel->getClientCount() > 0) {
        Client* newOp = *channel->getClients().begin();
        channel->addOperator(newOp);
        std::string modeMsg = ":server MODE " + channelName + " +o " + newOp->nickname();
        sendToChannel(channel, modeMsg);
        std::string noticeMsg = ":server NOTICE " + channelName + " :" + newOp->nickname() + " has been promoted to operator";
        sendToChannel(channel, noticeMsg);
        std::cout << "Client " << newOp->nickname() << " promoted to operator in " << channelName << std::endl;
    }

    if (channel->getClientCount() == 0){
      _server->removeChannel(channelName);
    } 

    std::cout << "Client " << nick << " left channel " << channelName << std::endl;
}

void CommandHandler::handleTopic(Client& client, const std::vector<std::string>& params)
{
  if (!client.isRegistered())
  {
    sendNumeric(client, 451, ":You have not registered");
    return ;
  }
  if (params.empty())
  {
    sendNumeric(client, 461, "TOPIC :Not enough parameters");
    return ;
  }
  std::string channelName = params[0];
  if (!isValidChannelName(channelName)) {
    sendNumeric(client, 403, channelName + " :No such channel");
    return ;
  }
  Channel* channel = _server->getChannel(channelName);
  if (!channel) {
    sendNumeric(client, 403, channelName + " :Channel does not exist");
    return ;
  }
  if (!channel->hasClient(&client)) {
    sendNumeric(client, 442, channelName + " :You're not on that channel");
    return ;
  }
  std::string nick = client.nickname();
  if (params.size() == 1) {
    if (channel->getTopic().empty()) {
      sendNumeric(client, 331, channelName + " :No topic is set on this channel");
    } else {
      sendNumeric(client, 332, channelName + " :" + channel->getTopic());
    }
    return ;
  }
  std::string newTopic;
  for (size_t i = 1; i < params.size(); ++i) {
    if (i > 1)
      newTopic += " ";
    newTopic += params[i];
  }
  if (channel->hasTopicRestriction() && !channel->isOperator(&client)) {
    sendNumeric(client, 482, channelName + " :You're not channel operator");
    return ;
  }
  channel->setTopic(newTopic);
  std::string user = client.username();
  std::string topicMsg = ":" + nick + "!" + user + "@localhost TOPIC " + channelName + " :" + newTopic;
  sendToChannel(channel, topicMsg);
  std::cout << "Topic for " << channelName << " changed by " << nick << " to: " << newTopic << std::endl;
}

void CommandHandler::handleKick(Client& client, const std::vector<std::string>& params) {
  if (!client.isRegistered()){
    sendNumeric(client, 451, ":You have not registered");
    return ;
  }
  if (params.size() < 2) { // Usage: KICK #channel nickname :reason
    sendNumeric(client, 461, "KICK :Not enough parameters");
    return ;
  }
  std::string channelName = params[0];
  std::string targetNick = params[1];
  std::string kickMessage = params.size() > 2 ? params[2] : client.nickname();
  if (!isValidChannelName(channelName)) {
    sendNumeric(client, 403, channelName + " :No such channel");
    return ;
  }
  Channel* channel = _server->getChannel(channelName);
  if (!channel) {
    sendNumeric(client, 403, channelName + " :No such channel");
    return ;
  }
  if (!channel->hasClient(&client)) {
    sendNumeric(client, 442, channelName + " :You're not on that channel");
    return ;
  }
  if (!channel->isOperator(&client)) { // the moderator is the only one allowed to kick someone
    sendNumeric(client, 482, channelName + " :You're not channel operator");
    return ;
  }
  Client* targetClient = nullptr; // target user to kick
  for (Client* c : channel->getClients()) {
    if (c && c->nickname() == targetNick) {
      targetClient = c;
      break ;
    }
  }
  if (!targetClient) { // target must be present in the channel
    sendNumeric(client, 401, targetNick + " :No such nick/channel");
    return ;
  }
  if (targetClient == &client) { // moderator cannot kick himself
    sendNumeric(client, 484, channelName + " :Cannot kick yourself");
    return ;
  }
  std::string kickMsg = ":" + client.nickname() + "!" + client.username() + "@localhost KICK " + channelName + " " + targetNick;
  if (!kickMessage.empty())
    kickMsg += " :" + kickMessage;
  sendToChannel(channel, kickMsg);
  channel->removeClient(targetClient);
  if (channel->getClientCount() == 0) {
    _server->removeChannel(channelName);
  }

    std::cout << "Client " << client.nickname() << " kicked " << targetNick << " from " << channelName << std::endl;
}

void CommandHandler::handleInvite(Client& client, const std::vector<std::string>& params) {
  if (!client.isRegistered()) {
    sendNumeric(client, 451, ":You have not registered");
    return ;
  }
  if (params.size() < 2) { // usage INVITE <nickname> <channel>
    sendNumeric(client, 461, "INVITE :Not enough parameters");
    return ;
  }
  std::string targetNick = params[0];
  std::string channelName = params[1];
  if (!isValidChannelName(channelName)) {
    sendNumeric(client, 403, channelName + " :No such channel");
    return ;
  }
  Channel* channel = _server->getChannel(channelName);
  if (!channel) {
    sendNumeric(client, 403, channelName + " :No such channel");
    return ;
  }
  if (!channel->hasClient(&client)) {
    sendNumeric(client, 442, channelName + " :You're not on that channel");
    return ;
  }
  if (!channel->isOperator(&client)) {
    sendNumeric(client, 482, channelName + " :You are not channel operator");
    return ;
  }
  Client* targetClient = nullptr;
  for (auto& clientPair : _server->getClients()) {
    if (clientPair.second.nickname() == targetNick) {
      targetClient = const_cast<Client*>(&clientPair.second);
      break ;
    }
  }
  if (!targetClient) {
    sendNumeric(client, 401, targetNick + " :No such nick or channel");
    return ;
  }
  if (channel->hasClient(targetClient)) { //Cannot invite people that are already in the channel
    sendNumeric(client, 443, targetNick + " " + channelName + " :is already on channel");
    return ;
  }
  channel->addInvitedClient(targetClient);
  sendNumeric(client, 341, targetNick + " " + channelName);
  std::string inviteMsg = ":" + client.nickname() + "!" + client.username() + "@localhost INVITE " + targetNick + " " + channelName;
  sendToClient(*targetClient, inviteMsg);
  std::cout << "Client " << client.nickname() << " invited " << targetNick << " to channel " << channelName << std::endl;
}
