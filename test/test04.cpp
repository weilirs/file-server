#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"
#include <bits/stdc++.h>

using std::cout;

// FS_DELETE Testing
int main(int argc, char *argv[])
{
    char *server;
    int server_port;

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];
    int status;

    if (argc != 3)
    {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);

    status = fs_create("user1", "/dir", 'd');
    assert(!status);

    status = fs_create("user1", "/dir/file", 'f');
    assert(!status);

    status = fs_writeblock("user1", "/dir/file", 0, writedata);
    assert(!status);

    // can't delete directory with files
    status = fs_delete("user1", "/dir");
    assert(status);

    status = fs_delete("user1", "/dir/file");
    assert(!status);

    // it should be an invalid read
    status = fs_readblock("user1", "/dir/file", 0, readdata);
    assert(status);

    // delete invalid pathname
    status = fs_delete("user1", "/dir/file");
    assert(status);

    // delete with invalid username
    status = fs_delete("user2", "/dir");
    assert(status);

    // delete root
    status = fs_delete("user1", "/");
    assert(status);

    // delete empty directory
    status = fs_delete("user1", "/dir");
    assert(!status);

    // check the shrinking and shifting by FS_DELETE
    for (int i = 0; i < 20; i++)
    {
        char num_char[6];
        std::sprintf(num_char, "%d", i);
        char cat[20] = "/file";
        status = fs_create("user1", strcat(cat, num_char), 'f');
    }

    for (int i = 8; i < 16; i++)
    {
        char num_char[6];
        std::sprintf(num_char, "%d", i);
        char cat[20] = "/file";
        status = fs_delete("user1", strcat(cat, num_char));
    }

    // // delete the only dir_entry in the newly allocated disk block
    // status = fs_delete("user1", "/dir8");
    // assert(!status);
}
