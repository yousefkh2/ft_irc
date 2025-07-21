#pragma once
#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"
#include <string>
#include <vector>
#include <poll.h>
#include <unordered_map>

class Server {
public:
    Server(int port, const std::string& password);
	~Server();

    int run(); // will start the event loop (poll/select)
    void stop(); // gracefully stop the loop

	// Channel management
	Channel* getChannel(const std::string& name);
	Channel* createChannel(const std::string& name);
	void removeChannel(const std::string& name);
    bool channelExists(const std::string& name) const;
	const std::unordered_map<int, Client>& getClients() const;
	bool isNicknameInUse(const std::string& nickname) const;
	//broadcasting
	void broadcastToClientChannels(Client* client, const std::string& message, CommandHandler& handler);
	void disconnectClient(int fd);
	
	// forbid copying and assignment
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

private:
	void initSocket();
	void acceptNewClient();
	void handleClientData(size_t idx);
	void cleanup();
	void setNonBlocking(int fd);
	void removeClientFromAllChannels(Client* client);  // Clean up client from all channels

    int _port;
    std::string     _password;
	int				_server_fd;
	std::vector<pollfd> _fds;
	std::unordered_map<int, Client> _clients;
	std::unordered_map<std::string, Channel> _channels;
	CommandHandler		_handler;
};

