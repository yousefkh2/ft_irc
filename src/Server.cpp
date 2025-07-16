
#include "../include/Server.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include "../include/Parser.hpp"

Server::Server(int port, const std::string& password)
	: _port(port), _password(password), _server_fd(-1), _handler(password)
	{
		initSocket(); // later
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
	setsockopt(
		_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // allow re-binding after the socket closes without waiting
  
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(_port);
  
	if (bind(_server_fd, reinterpret_cast<sockaddr *>(&server_addr),
			 sizeof(server_addr)) < 0) {
	  perror("bind");
	  throw std::runtime_error("Failed to bind socket");
	}
  
	if (listen(_server_fd, 5) < 0) {
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
	_fds.push_back(listen_pollfd);  // created first entry
}

int Server::run() {
	while (true) {
		int ret = poll(_fds.data(), _fds.size(), -1);
    if (ret < 0) {
      perror("poll");
      return 1;
    }

    for (size_t i = 0; i < _fds.size(); ++i) {
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
	int client_fd =
		accept(_server_fd, reinterpret_cast<sockaddr *>(&client_addr),
			   &client_addr_len);
	if (client_fd < 0) {
	  perror("accept");
	} else {
	  char ip[16];
	  inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
	  std::cout << "New connection: " << ip << ":"
				<< ntohs(client_addr.sin_port) << std::endl;
		_clients[client_fd] = Client(client_fd);

	  pollfd client_pollfd;
	  client_pollfd.fd = client_fd;
	  client_pollfd.events = POLLIN;
	  client_pollfd.revents = 0;
	  _fds.push_back(client_pollfd);
	}
}
      
void Server::handleClientData(size_t idx)
{
	int fd = _fds[idx].fd;
	char buffer[512]; // 512 is max len of a single IRC message as per IRC 1459
	int n = recv(fd, buffer, sizeof(buffer), 0); // recv is the network version of read()
	if (n <= 0) {
	  std::cout << "Client " << fd << " disconnected\n";
	  close(fd);
	  _fds.erase(_fds.begin() + idx);
	  _clients.erase(fd);
	  return;
	} 

	Client& client = _clients[fd];
	std::string& buf = client.buffer(); 
	buf.append(buffer, n); // now it's filled!

	Parser parser;
	size_t used = 0;
	std::vector<Command> cmds = parser.parse(buf, used);

	buf.erase(0, used); // erases from position 0 (beginning of string) up to position used (erases complete commands)
	for (auto& cmd : cmds)
	{
		_handler.handle(client, cmd);
		std::cout << "Parsed command: " << cmd.name;
		if (!cmd.params.empty()) {
			std::cout << " [";
			for (auto& p : cmd.params) std::cout << p << " ";
			std::cout << "]";
		}
		std::cout << std::endl;
	}
}

// Returns pointer to existing channel, or nullptr if not found
Channel* Server::getChannel(const std::string& name)
{
	auto it = _channels.find(name);
	if (it != _channels.end())
		return &it->second;
	return nullptr;
}

// Creates a new channel with given name and returns pointer to it
Channel* Server::createChannel(const std::string& name)
{
	// Check if channel already exists
	if (channelExists(name))
		return getChannel(name);
	// Create new channel using emplace (constructs in-place)
	auto result = _channels.emplace(name, Channel(name));
	std::cout << "Created new channel: " << name << std::endl;
	return &result.first->second;
}

// Removes a channel if it exists and is empty
void Server::removeChannel(const std::string& name)
{
	auto it = _channels.find(name);
	if (it != _channels.end())
	{
		Channel& channel = it->second;
		if (channel.getClientCount() == 0) // Only remove if channel is empty
		{
			std::cout << "Removing empty channel: " << name << std::endl;
			_channels.erase(it);
		}
	}
}

// Checks if a channel exists
bool Server::channelExists(const std::string& name) const
{
	return _channels.find(name) != _channels.end();
}

void Server::cleanup() {
	for (auto& pfd : _fds) {
		close(pfd.fd);
	}
	_fds.clear();
}
        
