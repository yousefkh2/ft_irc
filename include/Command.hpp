#pragma once
#include <string>
#include <vector>

struct Command {
	std::string prefix;			// e.g. "Nick!user@host" [this is optional!]
    std::string name;            // e.g. "NICK", "USER", "PRIVMSG"
    std::vector<std::string> params; // tokens or trailing param
};
