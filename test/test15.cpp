#include <iostream>
#include <array>
#include <string_view>
#include <unistd.h>
#include "fs_client.h"

// building up a single user linux

using std::cout;
using namespace std::string_view_literals;

int main(int argc, char *argv[])
{
    char *server;
    int server_port;

    if (argc != 3)
    {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        return 1;
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    std::array<char, 512> data;
    data.fill(0);
    fs_create("manosk", "/home", 'd');
    fs_create("manosk", "/home/manosk", 'd');
    fs_create("manosk", "/home/manosk/.profile", 'f');
    "#!/bin/bash\nexport PATH=$PATH:/home/manosk/.local/bin"sv.copy(data.data(), 512);
    fs_writeblock("manosk", "/home/manosk/.profile", 0, data.data());
    fs_readblock("manosk", "/home/manosk/.profile", 0, data.data());
    fs_create("manosk", "/file", 'f');
    fs_writeblock("manosk", "/file", 0, data.data());
    fs_readblock("barisk", "/file", 0, data.data());
    fs_delete("barisk", "/file");
    fs_create("manosk", "/home/manosk/.local", 'd');
    fs_create("manosk", "/home/manosk/.local/bin", 'd');
    fs_create("barisk", "/home/barisk", 'd');
    fs_create("barisk", "/barisk", 'd');
    fs_create("barisk", "/barisk/home", 'd');
    fs_create("barisk", "/barisk/home/", 'd');
    fs_create("barisk", "/barisk///home", 'd');
    fs_delete("barisk", "/barisk");
    fs_delete("manosk", "/home/manosk/.local");
    fs_delete("manosk", "/home/manosk/.local/bin");
    fs_delete("manosk", "/home/manosk/.profile");
    return 0;
}