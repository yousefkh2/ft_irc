#pragma once
#include "Client.hpp"
#include <string>
#include <vector>
#include <poll.h>
#include <unordered_map>
#include "CommandHandler.hpp"


class Server {
public:
    Server(int port, const std::string& password);
	~Server();

    int run(); // will start the event loop (poll/select)
    void stop(); // gracefully stop the loop

	// forbid copying and assignment
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

private:
	void initSocket();
	void acceptNewClient();
	void handleClientData(size_t idx);
	void cleanup();

    int _port;
    std::string     _password;
	int				_server_fd;
	std::vector<pollfd> _fds;
	std::unordered_map<int, Client> _clients;
	CommandHandler		_handler;
};

