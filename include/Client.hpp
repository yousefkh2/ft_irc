#pragma once

#include <string>

class Client {
	public:
	Client();
	Client(int fd);
	int getFd() const;
	std::string& buffer();


	private:
	int _fd;
	std::string _buffer;
};