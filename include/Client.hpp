#pragma once
#include <string>

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

	// registration flags
	bool hasPassed() const;
	void setPassed(bool);

	bool hasNick() const;
	void setNickSet(bool);

	bool hasUser() const;
	void setUserSet(bool);

	bool isRegistered() const;

private:
	int _fd;
	std::string _buffer;

	std::string _nickname;
	std::string _username;

	bool _passed = false;
	bool _nickSet = false;
	bool _userSet = false;
};