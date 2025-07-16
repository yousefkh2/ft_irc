#include "../include/Server.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <algorithm>
#include "../include/Parser.hpp"

Server::Server(int port, const std::string& password)
	: _port(port), _password(password), _server_fd(-1), _handler(password)
	{
		initSocket();
	}

Server::~Server() {
	cleanup();
}

void Server::initSocket() {
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0) {
	  perror("socket");
	  throw std::runtime_error("Failed to create socket");
	}
  
	int opt = 1;
	setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(_port);
  
	if (bind(_server_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
	  perror("bind");
	  close(_server_fd);
	  throw std::runtime_error("Failed to bind socket");
	}
  
	// FIXED: Changed from < 128 to < 0
	if (listen(_server_fd, 5) < 0) {
	  perror("listen");
	  close(_server_fd);
	  throw std::runtime_error("Failed to listen on socket");
	}
  
	std::cout << "Server listening on port " << _port << std::endl;
  
	// prepare pollfd vector: first entry is the listening socket
	pollfd listen_pollfd;
	listen_pollfd.fd = _server_fd;
	listen_pollfd.events = POLLIN;
	listen_pollfd.revents = 0;
	_fds.push_back(listen_pollfd);
}

int Server::run() {
	while (true) {
		int ret = poll(_fds.data(), _fds.size(), -1);
		if (ret < 0) {
			perror("poll");
			return 1;
		}

		// Process events (iterate backwards to handle removals safely)
		for (int i = _fds.size() - 1; i >= 0; --i) {
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
	sockaddr_in client_addr{};
	socklen_t client_addr_len = sizeof(client_addr);
	int client_fd = accept(_server_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_addr_len);
	
	if (client_fd < 0) {
		perror("accept");
		return;
	}

	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
	std::cout << "New connection: " << ip << ":" << ntohs(client_addr.sin_port) << std::endl;
	
	_clients[client_fd] = Client(client_fd);

	pollfd client_pollfd;
	client_pollfd.fd = client_fd;
	client_pollfd.events = POLLIN;
	client_pollfd.revents = 0;
	_fds.push_back(client_pollfd);
}
      
void Server::handleClientData(size_t idx) {
	int fd = _fds[idx].fd;
	char buffer[512];
	int n = recv(fd, buffer, sizeof(buffer) - 1, 0);
	
	if (n <= 0) {
		std::cout << "Client " << fd << " disconnected\n";
		close(fd);
		_fds.erase(_fds.begin() + idx);
		_clients.erase(fd);
		return;
	}

	buffer[n] = '\0'; // Null-terminate for safety
	
	auto client_it = _clients.find(fd);
	if (client_it == _clients.end()) {
		std::cerr << "Client " << fd << " not found in clients map\n";
		return;
	}
	
	Client& client = client_it->second;
	std::string& buf = client.buffer();
	buf.append(buffer, n);

	Parser parser;
	size_t used = 0;
	std::vector<Command> cmds = parser.parse(buf, used);

	buf.erase(0, used);
	
	for (auto& cmd : cmds) {
		_handler.handle(client, cmd);
		std::cout << "Parsed command: " << cmd.name;
		if (!cmd.params.empty()) {
			std::cout << " [";
			for (size_t i = 0; i < cmd.params.size(); ++i) {
				if (i > 0) std::cout << ", ";
				std::cout << "'" << cmd.params[i] << "'";
			}
			std::cout << "]";
		}
		std::cout << std::endl;
	}
}

void Server::cleanup() {
	for (auto& pfd : _fds) {
		if (pfd.fd >= 0) {
			close(pfd.fd);
		}
	}
	_fds.clear();
	_clients.clear();
}

void Server::stop() {
	// Graceful shutdown - close all connections
	cleanup();
}
