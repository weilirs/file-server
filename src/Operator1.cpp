// #include "Operator.h"
// #include <cstring>

// Operator::Operator(Server &_server, int _client, FileSystem &_filesystem) : server(_server), client(_client),
//                                                                             request(), filesystem(_filesystem), data()
// {
// }

// void Operator::parse(void)
// {
//     std::array<char, 1024> buffer;
//     buffer.fill(0);
//     char *buffer_iterator = buffer.data();
//     int total_size = 0;
//     total_size = server.receive(client, buffer_iterator, buffer.size());
//     request.parse(std::string_view(buffer.data(), buffer_iterator - buffer.data()));
//     if (request.command == FS_WRITEBLOCK)
//     {
//         // int temp = buffer.data() - ++buffer_iterator;
//         int remaining = FS_BLOCKSIZE - (buffer.data() + total_size - ++buffer_iterator);
//         int remaining1 = FS_BLOCKSIZE - (buffer.data() - ++buffer_iterator + total_size);
//         int received = -1;
//         received = server.receiveUntil(client, buffer.data() + total_size, remaining);
//         if (received < remaining)
//         {
//             throw std::runtime_error("receive error");
//         }
//         data.reset(new BlockData());
//         std::memcpy(data.get(), buffer_iterator, data->size());
//     }
//     if (request.command == FS_READBLOCK)
//     {
//         data.reset(new BlockData());
//     }
// }

// void Operator::operator()(void)
// {
//     try
//     {
//         parse();
//     }
//     catch (std::exception &err)
//     {
//         std::cerr << err.what() << std::endl;
//         close(client);
//         return;
//     }
//     int return_value = -1;
//     switch (request.command)
//     {
//     case FS_READBLOCK:
//         return_value = filesystem.readFileBlock(request, data);
//         break;
//     case FS_WRITEBLOCK:
//         return_value = filesystem.writeFileBlock(request, data);
//         break;
//     case FS_CREATE:
//         return_value = filesystem.createEntry(request);
//         break;
//     case FS_DELETE:
//         return_value = filesystem.deleteEntry(request);
//         break;
//     default:
//         break;
//     }
//     if (return_value == 0)
//     {
//         server.send(client, request.messageView());
//         if (request.command == FS_READBLOCK)
//             server.send(client, std::string_view(data->data(), FS_BLOCKSIZE), false);
//     }
//     close(client);
// }