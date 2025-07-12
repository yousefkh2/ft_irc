
#include <arpa/inet.h>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/_types/_fd_def.h>
#include <sys/_types/_socklen_t.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
    return 1;
  }
  int port = std::stoi(argv[1]);


  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("socket");
    return 1;
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // allow re-binding after the socket closes without waiting

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
    perror("bind");
    close(server_fd);
    return 1;
  }

  if (listen(server_fd, 5) < 0) {
    perror("listen");
    close(server_fd);
    return 1;
  }

  std::cout << "Server listening on port " << port << std::endl;

  // prepare pollfd vector: first entry is the listening socket
  std::vector<pollfd> fds;
  pollfd listen_pollfd;
  listen_pollfd.fd = server_fd;
  listen_pollfd.events = POLLIN; // request notification when ready to accept (incoming connection) or has data to read
  listen_pollfd.revents = 0; // no events yet
  fds.push_back(listen_pollfd); // created first entry

  while (true) {
	int ret = poll(fds.data(), fds.size(), -1);
	if (ret < 0) {
		perror("poll");
		break;
	}

	for (size_t i = 0; i < fds.size(); ++i)
	{
		bool has_incoming_data = (fds[i].revents & POLLIN);
		if (!has_incoming_data) continue;

		if (fds[i].fd == server_fd) {
			// new connection from a client
			sockaddr_in client_addr{};
			socklen_t client_addr_len = sizeof(client_addr);
			int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
			if (client_fd < 0) {
				perror("accept");
			} else {
				char ip[16];
				inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
				std::cout << "New connection: " << ip << ":" << ntohs(client_addr.sin_port) << std::endl;
				pollfd client_pollfd;
				client_pollfd.fd = client_fd;
				client_pollfd.events = POLLIN;
				client_pollfd.revents = 0;
				fds.push_back(client_pollfd);
			}
		} else {
			// data from existing client
			char buffer[512]; // 512 is max len of a single IRC message as per IRC 1459
			int n = recv(fds[i].fd, buffer, sizeof(buffer), 0); // recv is the network version of read()
			if (n <= 0) {
				// disconnect
				std::cout << "Client " << fds[i].fd << " disconnected\n";
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				--i; // adjust index after erase
			} else {
				std::cout << "Received" << n << " bytes from client " << fds[i].fd << std::endl;
				// we log that we received a message but we ignore the content for now
			}
		}
	}
  }


for (auto &pfd : fds) close (pfd.fd);
return 0;
}
