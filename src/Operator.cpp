#include "Operator.h"
#include <cstring>

Operator::Operator(Server &_server, int _client, FileSystem &_filesystem) : server(_server), client(_client),
                                                                            request(), filesystem(_filesystem), data()
{
}

void Operator::parse(void)
{
    std::array<char, FS_MAXREQUESTMSG + FS_BLOCKSIZE> buffer;
    buffer.fill(0);
    char *buffer_iterator = buffer.data();
    int total_size = 0;
    total_size = server.receive(client, buffer_iterator, buffer.size());
    request.parse(std::string_view(buffer.data(), buffer_iterator - buffer.data()));
    if (request.command == FS_READBLOCK || request.command == FS_WRITEBLOCK)
    {
        data.reset(new BlockData());
    }
    if (request.command == FS_WRITEBLOCK)
    {
        int remaining = FS_BLOCKSIZE - (buffer.data() + total_size - ++buffer_iterator);
        int received = -1;
        received = server.receiveUntil(client, buffer.data() + total_size, remaining);
        if (received < remaining)
        {
            throw std::runtime_error("receive error");
        }
        std::memcpy(data.get(), buffer_iterator, data->size());
    }
}

void Operator::operator()(void)
{
    try
    {
        parse();
        switch (request.command)
        {
        case FS_READBLOCK:
            filesystem.readFileBlock(request, data);
            break;
        case FS_WRITEBLOCK:
            filesystem.writeFileBlock(request, data);
            break;
        case FS_CREATE:
            filesystem.createEntry(request);
            break;
        case FS_DELETE:
            filesystem.deleteEntry(request);
            break;
        default:
            break;
        }
    }
    catch (const std::exception &err)
    {
#ifdef DEBUG
        cout_lock.lock();
        std::cerr << err.what() << std::endl;
        std::cerr << "Request: " << request.messageView() << " failed" << std::endl;
        cout_lock.unlock();
#endif
        close(client);
        return;
    }
    server.send(client, request.messageView().data(), request.messageView().length() + 1);
    if (request.command == FS_READBLOCK)
        server.send(client, data->data(), FS_BLOCKSIZE);
    close(client);
}