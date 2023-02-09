#pragma once

#include "fs_server.h"
#include "Request.h"
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

class Server
{
protected:
    sockaddr_in address;
    int server;

public:
    static inline Server *self = nullptr;
    static void signalExitHandler(int _signal);
    Server(unsigned int port = 0);
    int start(void);
    int receive(int client, char *&buffer, size_t buffer_size);
    int receiveUntil(int client, char *buffer, size_t buffer_size);
    int send(int client, const char *data, size_t length);
    int accept(void);
    ~Server();
};