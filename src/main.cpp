#include "../include/Server.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <csignal>



void signalHandler(int signum) {
  (void)signum;
}

int main(int argc, char *argv[]) {
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
      return 1;
  }
  int port = std::stoi(argv[1]);
  std::string password = argv[2];
  Server srv(port, password);
  return srv.run();
}
