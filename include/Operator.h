#pragma once

#include "Server.h"
#include "Request.h"
#include "FileSystem.h"
#include <memory>

class Operator
{
protected:
    Server &server;
    int client;
    Request request;
    FileSystem &filesystem;
    std::unique_ptr<BlockData> data;

    void parse(void);

public:
    Operator(Server &_server, int _client, FileSystem &_filesystem);
    void operator()(void);
};