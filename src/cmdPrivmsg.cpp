#include "../include/CommandHandler.hpp"
#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include "../include/Utils.hpp"

static std::vector<std::string> splitString(const std::string& str, char delimiter) {
	std::vector<std::string> result;
	std::string::size_type start = 0;

	while (start < str.size()) {
		while (start < str.size() && str[start] == delimiter)
			++start;
		auto end = str.find(delimiter, start);
		if (end == std::string::npos)
			end = str.size();
		if (start < end)
			result.emplace_back(str.substr(start, end - start));
			
		start = end + 1;
	}
	return result;
}

void CommandHandler::handlePrivmsg(Client& client, const std::vector<std::string>& params)
{
	if (!client.isRegistered())
		return ;
	if (params.size() < 2)
	{
		sendNumeric(client, 411, ":No recipient given (PRIVMSG)");	
		return ; // ERR_NORECIPIENT
	}
	if (params[1].empty())
	{
		sendNumeric(client, 412, ":No text to send");	
		return ; // ERR_NOTEXTTOSEND
	}
	/* SPECIAL TARGETS - probably out of scope too*/
	// *
	// Use the active nickname or channel
	// ,
	// Last person who sent you a /msg
	// .
	// Last person you sent a /msg to

	std::vector<std::string> targets = splitString(params[0], ',');
	if (targets.empty())
	{
		sendNumeric(client, 411, ":No recipient given (PRIVMSG)"); // ERR_NORECIPIENT
		return ;
	}
	
	for (const std::string& target : targets)
	{
		if (target[0] == '#') // send message to channel
		{
			Channel* channel = _server->getChannel(target);
			if (!channel) {
				sendNumeric(client, 404, target + " :No such channel"); // ERR_CANNOTSENDTOCHAN
				continue ;
			}
			std::string fullMsg = ":" + client.nickname() + "!" + 
								client.username() + "@" + client.hostname() + " PRIVMSG " + target + " :" + params[1] + "\r\n";
			// send to all channel members except sender
			for (const auto& member : channel->getClients()) {

					sendRaw(*member, fullMsg);
				}
			}
		else // message to client
		{
			Client* targetClient = nullptr;
			for (const auto& pair : _server->getClients()) {
				if (pair.second.nickname() == target) {
					targetClient = const_cast<Client*>(&pair.second);
					break ;
				}
			}
			if (!targetClient) {
				sendNumeric(client, 401, target + " :No such nick/channel"); // ERR_NOSUCHNICK
				continue ;
			}
			std::string fullMsg = ":" + client.nickname() + "!" + 
								client.username() + "@" + client.hostname() + " PRIVMSG " + target + " :" + params[1] + "\r\n";
			sendRaw(*targetClient, fullMsg);
		}
	}
	/* OUT OF SCOPE */
	// If <target> is a channel name and the client is banned and not covered by a ban exception, the message will not be delivered and the command will silently fail.
	
	/* OUT OF SCOPE */
	// 	If `<target>` is a channel name, it may be prefixed with one or more [*channel membership prefix character (`@`, `+`, etc)*](https://modern.ircdocs.horse/#channel-membership-prefixes) and the message will be delivered only to the members of that channel with the given or higher status in the channel. Servers that support this feature will list the prefixes which this is supported for in the [`*STATUSMSG*`](https://modern.ircdocs.horse/#statusmsg-parameter) `RPL_ISUPPORT` parameter, and this SHOULD NOT be attempted by clients unless the prefix has been advertised in this token.

	/* OUT OF SCOPE */
	// If `<target>` is a user and that user has been set as away, the server may reply with an [`*RPL_AWAY*`](https://modern.ircdocs.horse/#rplaway-301) `(301)` numeric and the command will continue.
}