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
if (!isValidChannelName(channelName)) {
sendNumeric(client, 403, channelName + " :No such channel");
return;
}

// Validate client state more thoroughly
if (client.nickname().empty() || client.username().empty()) {
sendNumeric(client, 451, ":Registration incomplete");
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

channel->addClient(&client);

// Defensive programming for message construction
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
