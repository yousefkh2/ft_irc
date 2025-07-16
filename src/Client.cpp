#include "../include/Client.hpp"

Client::Client() : _fd(-1) {}
Client::Client(int fd) : _fd(fd) {}

int Client::getFd() const { return _fd; }

std::string &Client::buffer() { return _buffer; }

const std::string& Client::nickname() const { return _nickname; }
void Client::setNickname(const std::string& nick) { _nickname = nick; }

const std::string& Client::username() const { return _username; }
void Client::setUsername(const std::string& user) { _username = user; }

bool Client::hasPassed() const { return _passed; }
void Client::setPassed(bool v) { _passed = v; }

bool Client::hasNick() const { return _nickSet; }
void Client::setNickSet(bool v) { _nickSet = v; }

bool Client::hasUser() const { return _userSet; }
void Client::setUserSet(bool v) { _userSet = v; }

bool Client::isRegistered() const {
	return _passed && _nickSet && _userSet;
}

bool Client::wasWelcomed() const { return _welcomed; }
void Client::setWelcomed(bool welcomed) { _welcomed = welcomed; }
