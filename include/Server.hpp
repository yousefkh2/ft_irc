#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <poll.h>


namespace IRC {

class Client;   // forward-declare
class Channel;  // forward-declare

class Server {
public:
    // paramaterized ctor(port, pass)
    Server(unsigned short port, std::string& password);
	~Server();

    void run(); // will start the event loop (poll/select)
    void stop(); // gracefully stop the loop

	// forbid copying and assignment
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

private:
    unsigned short _port; // us bec. it's mostly as short as 6667
    std::string     _password;
	int				_listenFd; // fd returned by socket() + bind() + listen()

	std::vector<pollfd> _pollFds;
	std::unordered_map<int, Client> _clients;

	std::unordered_map<std::string, Channel> _channels;


};

} 
