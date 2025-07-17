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
  if (!isValidChannelName(
          channelName)) // Validate channel name (must start with # or &)
  {
    sendNumeric(client, 403, channelName + " :No such channel");
    return;
  }
  Channel *channel = _server->getChannel(channelName);
  if (!channel)
    channel = _server->createChannel(channelName);
  if (channel->hasClient(&client))
    return;
  channel->addClient(&client);

  // Send JOIN message to all clients in channel (including the joiner)
  std::string joinMsg = ":" + client.nickname() + "!" + client.username() +
                        "@localhost JOIN " + channelName;
  sendToChannel(channel, joinMsg);
  if (!channel->getTopic().empty()) // Send topic if it exists
    sendToClient(client, "332 " + client.nickname() + " " + channelName + " :" +
                             channel->getTopic());
  // Send names list (list of users in channel)
  std::string namesList =
      "353 " + client.nickname() + " = " + channelName + " :";
  for (Client *c : channel->getClients()) {
    if (channel->isOperator(c))
      namesList += "@"; // @ prefix for operators
    namesList += c->nickname() + " ";
  }
  sendToClient(client, namesList);
  sendToClient(client, "366 " + client.nickname() + " " + channelName +
                           " :End of /NAMES list");
  std::cout << "Client " << client.nickname() << " joined channel "
            << channelName << std::endl;
}