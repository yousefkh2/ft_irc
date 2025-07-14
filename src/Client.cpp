#include "../include/Client.hpp"


Client::Client() : _fd(-1) {}
Client::Client(int fd) : _fd(fd) {}

int Client::getFd() const { return _fd; }

std::string& Client::buffer() {
    return _buffer;
}
