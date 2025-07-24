#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include "Client.hpp"

class Channel
{
    public:
        Channel(const std::string& name);
    
        //basic channel infos

        const std::string& getName() const;
        const std::string& getTopic() const;
        void setTopic(const std::string& topic);

    
        void addClient(Client* client);       
        void removeClient(Client* client);
        bool hasClient(Client* client) const;
        const std::set<Client*>& getClients() const; 
        size_t getClientCount() const;

        bool isInviteOnly() const;
        void setInviteOnly(bool inviteOnly);   
        bool hasTopicRestriction() const;      
        void setTopicRestriction(bool restriction); 
        void addInvitedClient(Client* client);
        bool isInvited(Client* client) const;
        void removeInvitedClient(Client* client);

        // Operator management
        bool isOperator(Client* client) const; 
        void addOperator(Client* client);      
        void removeOperator(Client* client); 

        // Channel password part
	    bool hasKey() const;
	    const std::string& getKey() const;
	    void setKey(const std::string& key);
	    void removeKey();

	    // Channel user limit part
	    bool hasUserLimit() const;
	    size_t getUserLimit() const;
	    size_t setUserLimit(size_t limit);
	    void removeUserLimit();

    private:
        std::string _name;
        std::string _topic;
        std::string _key;
        size_t _userLimit = 0;
        std::set<Client*> _clients; // All clients currently in the channel
        std::set<Client*> _operators;
        std::set<Client*> _invitedClients;
        // Channel modes
        bool _inviteOnly = false;      // If true, only invited users can join
        bool _topicRestriction = true; // If true, only operators can change topic
        bool _hasKey = false;
        bool _hasUserLimit = false;
};
