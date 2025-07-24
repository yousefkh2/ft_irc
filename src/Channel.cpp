#include "../include/Channel.hpp"
#include <iostream>

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
    _invitedClients.erase(client);
}

bool Channel::hasClient(Client *client) const
{
    return _clients.find(client) != _clients.end();
}

const std::set<Client*>& Channel::getClients() const
{
    return _clients;
}

size_t Channel::getClientCount() const
{
    return _clients.size();
}

bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}

void Channel::setInviteOnly(bool inviteOnly)
{
    _inviteOnly = inviteOnly;
}

bool Channel::hasTopicRestriction() const
{
    return _topicRestriction;
}

void Channel::setTopicRestriction(bool restriction)
{
    _topicRestriction = restriction;
}

bool Channel::isOperator(Client* client) const
{
    return _operators.find(client) != _operators.end();
}

void Channel::addOperator(Client* client)
{
    if (hasClient(client))
        _operators.insert(client);
}

void Channel::removeOperator(Client* client)
{
    _operators.erase(client);
}

void Channel::addInvitedClient(Client* client) {
    _invitedClients.insert(client);
}

bool Channel::isInvited(Client* client) const {
    return _invitedClients.find(client) != _invitedClients.end();
}

void Channel::removeInvitedClient(Client* client) {
    _invitedClients.erase(client);
}

bool Channel::hasKey() const {
    return _hasKey;
}

const std::string& Channel::getKey() const {
    return _key;
}

void Channel::setKey(const std::string&key) {
    _key = key;
    _hasKey = true;
}

void Channel::removeKey() {
    _key.clear();
    _hasKey = false;
}

bool Channel::hasUserLimit() const {
    return _hasUserLimit;
}

size_t Channel::getUserLimit() const {
    return _userLimit;  // Return the actual user limit, not the boolean flag
}

size_t Channel::setUserLimit(size_t limit) {
    _userLimit = limit;
    _hasUserLimit = true;
    return _userLimit;
}

void Channel::removeUserLimit() {
    _userLimit = 0;
    _hasUserLimit = false;
}
