
#include "../include/Server.hpp"
#include "../include/CommandHandler.hpp"
#include "../include/Parser.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "../include/Parser.hpp"
#include "../include/Channel.hpp"

Server::Server(int port, const std::string &password)
		: _port(port), _password(password), _server_fd(-1), _handler(password, this) {
	initSocket();
}
Server::~Server() { cleanup(); }

void Server::setNonBlocking(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl F_SETFL");
        throw std::runtime_error("Failed to set socket non-blocking");
    }
}

void Server::initSocket() {
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0) {
		perror("socket");
		throw std::runtime_error("Failed to create socket");
	}
	
	int opt = 1;
	setsockopt(
			_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
			sizeof(opt)); // allow re-binding after the socket closes without waiting

	setNonBlocking(_server_fd);

	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(_port);

	if (bind(_server_fd, reinterpret_cast<sockaddr *>(&server_addr),
					 sizeof(server_addr)) < 0) {
		perror("bind");
		throw std::runtime_error("Failed to bind socket");
	}

	if (listen(_server_fd, SOMAXCONN) < 0) {
		perror("listen");
		throw std::runtime_error("Failed to listen on socket");
	}

	std::cout << "Server listening on port " << _port << std::endl;

	// prepare pollfd vector: first entry is the listening socket
	pollfd listen_pollfd;
	listen_pollfd.fd = _server_fd;
	listen_pollfd.events = POLLIN; // request notification when ready to accept
																 // (incoming connection) or has data to read
	listen_pollfd.revents = 0;     // no events yet
	_fds.push_back(listen_pollfd); // created first entry
}

int Server::run() {
	while (true) {
		int ret = poll(_fds.data(), _fds.size(), -1);
		if (ret < 0) {
			perror("poll");
			return 1;
		}

		// back to front to avoid index problems when removing disconnected clients
		for (int i = static_cast<int>(_fds.size()) - 1; i >= 0; --i) {
			bool has_incoming_data = (_fds[i].revents & POLLIN);
			if (!has_incoming_data)
				continue;

			if (_fds[i].fd == _server_fd) {
				acceptNewClient();
			} else {
				handleClientData(i);
			}
		}
	}
	return 0;
}

void Server::acceptNewClient() {
	// new connection from a client
	sockaddr_in client_addr{};
	socklen_t client_addr_len = sizeof(client_addr);
	int client_fd = accept(_server_fd, reinterpret_cast<sockaddr *>(&client_addr),
												 &client_addr_len);
	if (client_fd < 0) {
		perror("accept");
	} else {
		setNonBlocking(client_fd);
		char ip[16];
		inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
		std::cout << "New connection: " << ip << ":" << ntohs(client_addr.sin_port)
							<< std::endl;
		_clients[client_fd] = Client(client_fd);
		_clients[client_fd].setHostname(ip);

		pollfd client_pollfd;
		client_pollfd.fd = client_fd;
		client_pollfd.events = POLLIN;
		client_pollfd.revents = 0;
		_fds.push_back(client_pollfd);
	}
}

void Server::handleClientData(size_t idx) {
	int fd = _fds[idx].fd;
	char buffer[512]; // 512 is max len of a single IRC message as per IRC 1459
	int n = recv(fd, buffer, sizeof(buffer), 0);
	if (n <= 0) {
		std::cout << "Client " << fd << " disconnected\n";
	auto clientIt = _clients.find(fd);
		if (clientIt != _clients.end()) {
				if (clientIt->second.isRegistered()) {
					std::string quitMsg = ":" + clientIt->second.nickname() + "!" + clientIt->second.username() + "@" + clientIt->second.hostname() + " QUIT :Connection lost\r\n";
					broadcastToClientChannels(&clientIt->second, quitMsg, _handler);
				}
				removeClientFromAllChannels(&clientIt->second);
	}
		close(fd);
		_fds.erase(_fds.begin() + idx);
		_clients.erase(fd);
		return;
	}
		
	Client &client = _clients[fd];
	std::string &buf = client.buffer();
	buf.append(buffer, n); // now it's filled!

	Parser parser;
	size_t used = 0;
	std::vector<Command> cmds = parser.parse(buf, used);

	buf.erase(0, used); // erases from position 0 (beginning of string) up to position used (erases complete commands)
	for (auto &cmd : cmds) {
		// std::cout << "Parsed command: " << cmd.name;
		// if (!cmd.params.empty()) {
		// 	std::cout << " [";
		// 	for (auto &p : cmd.params)
		// 	std::cout << p << " ";
		// 	std::cout << "]";
		// }
		// std::cout << std::endl;
		_handler.handle(client, cmd);
	}
}

Channel* Server::getChannel(const std::string& name)
{
	auto it = _channels.find(name);
	if (it != _channels.end())
		return &it->second;
	return nullptr;
}

Channel* Server::createChannel(const std::string& name)
{
	if (channelExists(name))
		return getChannel(name);

	auto result = _channels.emplace(name, Channel(name));
	std::cout << "Created new channel: " << name << std::endl;
	return &result.first->second; // iterator->second is the Channel object
}

void Server::removeChannel(const std::string& name)
{
	auto it = _channels.find(name);
	if (it != _channels.end())
	{
		Channel& channel = it->second;
		if (channel.getClientCount() == 0)
		{
			std::cout << "Removing empty channel: " << name << std::endl;
			_channels.erase(it);
		}
	}
}

void Server::removeClientFromAllChannels(Client* client) {
		for (auto& channelPair : _channels) {
				Channel& channel = channelPair.second;
				channel.removeClient(client);
		}
}

bool Server::channelExists(const std::string& name) const {
	return _channels.find(name) != _channels.end();
}

bool Server::isNicknameInUse(const std::string& nickname) const {
	for (const auto& pair : _clients) {
		if (pair.second.nickname() == nickname)
			return true ;
	}
	return false ;
}

void Server::disconnectClient(int fd) {
	auto clientIt = _clients.find(fd);
	if (clientIt != _clients.end()) {
		std::cout << "Disconnecting client " << fd << " (authentication failed)" << std::endl;
		removeClientFromAllChannels(&clientIt->second);
	
	// remove from poll array too
	for (auto it = _fds.begin(); it != _fds.end(); ++it) {
		if (it->fd == fd) {
			_fds.erase(it);
			break;
		}
	}
	close(fd);
	_clients.erase(clientIt);
	}
}

void Server::cleanup() {
	for (auto &pfd : _fds) {
		close(pfd.fd);
	}
	_fds.clear();
	_clients.clear();
}

const std::unordered_map<int, Client>& Server::getClients() const {
	return _clients;
}


void Server::broadcastToClientChannels(Client * client, const std::string& message, CommandHandler& handler)
{
	for (auto& channelPair : _channels) {
		Channel& channel = channelPair.second;
		if (channel.hasClient(client)) {
			for (Client* c : channel.getClients()) {
				if (c != client)
					handler.sendRaw(*c, message);
			}
		}
	}
}
