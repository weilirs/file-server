#include "Server.h"
#include "Request.h"
#include "FSBlock.h"
#include <cstring>
#include <sstream>
#include <string_view>

Server::Server(unsigned int port)
{
    self = this;
    std::memset(&address, 0, sizeof(sockaddr_in));
    server = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, NULL, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    int serverPort = !port;
    do
    {
        address.sin_port = htons(port);
        bind(server, (const sockaddr *)(&address), sizeof(address));
        socklen_t length = sizeof(address);
        getsockname(server, (sockaddr *)(&address), &length);
        serverPort = ntohs(address.sin_port);
    } while (port && serverPort != static_cast<int>(port));
    std::cout << "\n@@@ port " << serverPort << std::endl;
}

Server::~Server()
{
    close(server);
}

int Server::start(void)
{
    return listen(server, 30);
}

void Server::signalExitHandler(int _signal)
{
    close(self->server);
    exit(_signal);
}

int Server::accept(void)
{
    return ::accept(server, NULL, NULL);
}

int Server::receive(int client, char *&buffer, size_t buffer_size)
{
    size_t total_size = 0;
    int received_size = 0;
    do
    {
        received_size = recv(client, buffer + total_size, buffer_size - total_size, 0);
        if (received_size == -1)
        {
            throw std::runtime_error("socket error!");
        }
        size_t length = strnlen(buffer + total_size, received_size);
        total_size += received_size;
        if (length < static_cast<size_t>(received_size))
        {
            buffer += total_size - received_size + length;
            break;
        }
    } while (received_size > 0 && total_size < buffer_size);
    return total_size;
}

int Server::receiveUntil(int client, char *buffer, size_t buffer_size)
{
    if (buffer_size == 0)
    {
        return 0;
    }
    int received_size = 0;
    size_t total_size = 0;
    do
    {
        received_size = recv(client, buffer + total_size, buffer_size - total_size, MSG_WAITALL);
        total_size += received_size;
    } while (received_size > 0 && total_size < buffer_size);
    if (received_size == -1)
    {
        throw std::runtime_error("socket error!");
    }
    return received_size;
}

int Server::send(int client, const char *data, size_t length)
{
    size_t total_sent_length = 0;
    do
    {
        int bytes_sent = ::send(client, data + total_sent_length, length - total_sent_length, 0);
        if (bytes_sent == -1)
        {
            throw std::runtime_error("socket error!");
        }
        total_sent_length += bytes_sent;
    } while (total_sent_length < length);
    return total_sent_length;
}