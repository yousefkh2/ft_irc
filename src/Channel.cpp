#include "../include/Channel.hpp"

// Constructor
Channel::Channel(const std::string& name) : _name(name) {}

const std::string& Channel::getName() const
{
    return _name;
}

const std::string& Channel::getTopic() const
{
    return _topic;
}

void Channel::setTopic(const std::string& topic)
{
    _topic = topic;
}

void Channel::addClient(Client* client)
{
    _clients.insert(client);
    if (_clients.size() == 1)
        _operators.insert(client);
}

void Channel::removeClient(Client* client)
{
    _clients.erase(client);
    _operators.erase(client);
}

