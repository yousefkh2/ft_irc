#include "../include/CommandHandler.hpp"

static std::vector<std::string> splitString(const std::string& str, char delimiter) {
	std::vector<std::string> result;
	std::string::size_type start = 0;

	while (start < str.size()) {
		while (start < str.size() && str[start] == delimiter)
			++start;
		auto end = str.find(delimiter, start);
		if (end == string::npos)
			end = str.size();
		if (start < end)
			result.emplace_back(str.substr(start, end - start));
			
		start = end + 1;
	}
	return result;
}

void CommandHandler::cmdPrivmsg(Client& client, const std::vector<std::string>& params)
{
	if (client.isRegistered() == false)
		return ;
	if (params.size() < 2)
		return ; // ERR_NORECIPIENT (411)
	// "<client> :No recipient given (<command>)"
	else if (params.size() == 2)
		return ; // ERR_NOTEXTTOSEND (412)
	
	/* SPECIAL TARGETS - probably out of scope too*/
	// *
	// Use the active nickname or channel
	// ,
	// Last person who sent you a /msg
	// .
	// Last person you sent a /msg to
	std::vector<std::string> targets = splitString(params[0], ',');
	if (targets.size() < 1)
		return ; // ERR_NORECIPIENT (411)
	// "<client> :No recipient given (<command>)"
	for (std::vector<std::string>::const_iterator it = targets.begin(); it != targets.end(); ++it)
	{
		std::string target = *it;
		if (target[0] == '#')
		{
			//loop through server _channels
			// ERR_CANNOTSENDTOCHAN (404) - if cannot send to channel
			// "<client> <channel> :No such channel"
		}
		else
		{
			//loop through server _clients
			// ERR_NOSUCHNICK (401) - if cannot send to user
			// "<client> <nickname> :No such nick/channel"

		}
	}
	/* OUT OF SCOPE */
	// If <target> is a channel name and the client is banned and not covered by a ban exception, the message will not be delivered and the command will silently fail.
	
	/* OUT OF SCOPE */
	// 	If `<target>` is a channel name, it may be prefixed with one or more [*channel membership prefix character (`@`, `+`, etc)*](https://modern.ircdocs.horse/#channel-membership-prefixes) and the message will be delivered only to the members of that channel with the given or higher status in the channel. Servers that support this feature will list the prefixes which this is supported for in the [`*STATUSMSG*`](https://modern.ircdocs.horse/#statusmsg-parameter) `RPL_ISUPPORT` parameter, and this SHOULD NOT be attempted by clients unless the prefix has been advertised in this token.

	/* OUT OF SCOPE */
	// If `<target>` is a user and that user has been set as away, the server may reply with an [`*RPL_AWAY*`](https://modern.ircdocs.horse/#rplaway-301) `(301)` numeric and the command will continue.
}