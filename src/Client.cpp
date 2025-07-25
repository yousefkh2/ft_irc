#include "../include/Channel.hpp"
#include "../include/Client.hpp"

Client::Client() : _fd(-1) {}
Client::Client(int fd) : _fd(fd) {}

int Client::getFd() const { return _fd; }

std::string &Client::buffer() { return _buffer; }

const std::string& Client::nickname() const {return _nickname;}

void Client::setNickname(const std::string& nick) { _nickname = nick;}

const std::string& Client::username() const {return _username;}
void Client::setUsername(const std::string& user) {_username = user;}

const std::string& Client::hostname() const { return _hostname; }
void Client::setHostname(const std::string& hostname) { _hostname = hostname; }

bool Client::hasPassed() const {return _passed; }
void Client::setPassed(bool v) {_passed = v;}

bool Client::hasNick() const {return _nickSet;}
void Client::setNickSet(bool v) {_nickSet = v;}

bool Client::hasUser() const {return _userSet; }
void Client::setUserSet(bool v) { _userSet = v; }

bool Client::isRegistered() const {
	return _passed && _nickSet && _userSet;
}

void Client::joinChannel(const std::string& channelName) {
    _channels.insert(channelName);
}

void Client::leaveChannel(const std::string& channelName) {
    _channels.erase(channelName);
}

bool Client::isInChannel(const std::string& channelName) const {
    return _channels.find(channelName) != _channels.end();
}

const std::set<std::string>& Client::getChannels() const {
    return _channels;
}

void Client::leaveAllChannels() {
    _channels.clear();
}
