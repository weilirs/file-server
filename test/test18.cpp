#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include "fs_client.h"

#include <string>
#include "unistd.h"

using std::cout;

int main(int argc, char *argv[])
{
    char *server;
    int server_port;
    int status = 0;

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    if (argc != 3)
    {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);

    status = fs_create("manosk", "/manosk", 'd');
    assert(!status);
    status = fs_create("manosk", "/manosk/note", 'f');
    assert(!status);
    status = fs_create("root", "/note", 'f');
    assert(!status);
    status = fs_writeblock("manosk", "/manosk/note", 1, writedata);
    assert(status);
    status = fs_writeblock("root", "/note", 1, writedata);
    assert(status);
    for (size_t i = 0; i < FS_MAXFILEBLOCKS; i++)
    {
        status = fs_writeblock("manosk", "/manosk/note", i, writedata);
        assert(!status);
        status = fs_writeblock("root", "/note", i, writedata);
        assert(!status);
    }
    status = fs_writeblock("manosk", "/manosk/note", FS_MAXFILEBLOCKS, writedata);
    assert(status);
    status = fs_writeblock("root", "/note", FS_MAXFILEBLOCKS, writedata);
    assert(status);
    return 0;
}
