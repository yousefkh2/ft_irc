
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/_types/_socklen_t.h>
#include <sys/socket.h>
#include <unistd.h>

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

  // accept a single client for now
  sockaddr_in client_addr{};
  socklen_t client_addr_len = sizeof(client_addr);
  int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
  if (client_fd < 0) {
	perror("accept");
	close(server_fd);
	return 1;
	}

	char client_ip[16]; 
	inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
	std::cout << "New connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
	close(client_fd);
	std::cout << "Connection closed" << std::endl;
	close(server_fd);
	return 0;
}
