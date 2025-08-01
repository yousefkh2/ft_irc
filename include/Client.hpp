#pragma once
#include <string>
#include <set>

class Client {
public:
	Client();
	Client(int fd);
	int getFd() const;
	std::string& buffer();

	const std::string& nickname() const;
	void setNickname(const std::string& nick);

	const std::string& username() const;
	void setUsername(const std::string& user);

	const std::string& hostname() const;
	void setHostname(const std::string& hostname);

	// registration flags
	bool hasPassed() const;
	void setPassed(bool);

	bool hasNick() const;
	void setNickSet(bool);

	bool hasUser() const;
	void setUserSet(bool);

	bool isRegistered() const;

	// Channel addings
	void joinChannel(const std::string& channelName);
	void leaveChannel(const std::string& channelName);
	void leaveAllChannels();
	bool isInChannel(const std::string& channelName) const;
	const std::set<std::string>& getChannels() const;

private:
	int _fd;
	std::string _buffer;

	std::string _nickname;
	std::string _username;
	std::string _hostname;

	bool _passed = false;
	bool _nickSet = false;
	bool _userSet = false;

	std::set<std::string> _channels;
};