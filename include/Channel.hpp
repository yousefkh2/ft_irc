#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include "Client.hpp"

class Channel
{
    public:
        // Constructor: creates a new channel with given name
        Channel(const std::string& name);
    
        //basic channel infos

        const std::string& getName() const;
        const std::string& getTopic() const;
        void setTopic(const std::string& topic);

        //Managing members inside the channel
        void addClient(Client* client);        // Adds a client to the channel (first becomes operator)
        void removeClient(Client* client);
        bool hasClient(Client* client) const;  // Checks if client is in this channel
        const std::set<Client*>& getClients() const; // Returns all clients in channel
        size_t getClientCount() const;

        // Channel modes (for future expansion)
        bool isInviteOnly() const;
        void setInviteOnly(bool inviteOnly);   // Sets invite-only mode on/off
        bool hasTopicRestriction() const;      // Returns true if only ops can change topic
        void setTopicRestriction(bool restriction); // Sets topic restriction on/off

        // Operator management
        bool isOperator(Client* client) const; // Checks if client has operator privileges
        void addOperator(Client* client);      // Gives operator privileges to client
        void removeOperator(Client* client);   // Removes operator privileges from client

    private:
        std::string _name;
        std::string _topic;
        std::set<Client*> _clients; // All clients currently in the channel
        std::set<Client*> _operators;
        // Channel modes
        bool _inviteOnly = false;      // If true, only invited users can join
        bool _topicRestriction = true; // If true, only operators can change topic
};