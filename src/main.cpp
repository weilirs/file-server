#include "Operator.h"
#include "Server.h"
#include "fs_server.h"
#include <signal.h>

int main(int argc, char *argv[]) {
#ifdef DEBUG
  // for port reuse
  signal(SIGINT, Server::signalExitHandler);
  signal(SIGKILL, Server::signalExitHandler);
  signal(SIGTERM, Server::signalExitHandler);
  signal(SIGQUIT, Server::signalExitHandler);
#endif

  unsigned int port = 0;
  if (argc > 1) {
    sscanf(argv[1], "%u", &port);
  }
  // maximum single request length: 148
  FileSystem filesystem;

  Server server(port);
  server.start();

  while (1) {
    int client = server.accept();
    Operator operatorThread(server, client, filesystem);
    std::thread(std::move(operatorThread)).detach();
  }
  return 0;
}
