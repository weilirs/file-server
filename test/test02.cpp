#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include "fs_client.h"
using std::cout;

// FS_CREATE Testing
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

    // create root
    status = fs_create("user1", "/", 'd');
    assert(status);

    // delete root
    status = fs_delete("user1", "/");
    assert(status);

    // exceed maximum username length
    status = fs_create("user11111111", "/new_file", 'f');
    assert(status);

    // boundary check
    status = fs_create("user111111", "/length7", 'd');
    assert(!status);

    status = fs_create("user111111", "/length7/dir_name_long_at_boundary__________________________________", 'd');
    assert(!status);

    status = fs_create("user111111", "/length7/dir_name_long_at_boundary__________________________________/file_name_long_at_boundary_________________________________", 'f');
    assert(!status);

    // exceed maximum filename length
    status = fs_create("user1", "/dir/filefilefilefilefilefilefilefilefilefilefilefilefilefilefilefilefilefile", 'f');
    assert(status);

    // exceed maximum pathname
    status = fs_create("user1", "/dir/file_name_long_at_boundary__________________________________________________________________________________________________", 'f');
    assert(status);

    // username has space
    status = fs_create("user1 ", "/dir/file", 'f');
    assert(status);

    // username has space
    status = fs_create(" user1", "/dir/file", 'f');
    assert(status);

    // aliasing
    status = fs_create("user1 /git", "d ir/file", 'f');
    assert(status);

    // pathname end with /
    status = fs_create("user1", "/dir/file/", 'f');
    assert(status);

    // pathname doesn't start with /
    status = fs_create("user1", "file", 'f');
    assert(status);

    // /der doesn't exist
    status = fs_create("user1", "/der/file", 'f');
    assert(status);

    // user2 doesn't own /dir
    status = fs_create("user2", "/dir/file", 'f');
    assert(status);

    // create duplicate directory
    status = fs_create("user1", "/dir", 'd');
    assert(status);

    // path name has space
    status = fs_create("user1", "/di d", 'd');
    assert(status);

    // path name has space
    status = fs_create("user1", "/file f", 'f');
    assert(status);

    status = fs_create("user1", "/dir/file", 'f');
    assert(!status);

    status = fs_create("user1", "/dir/file", 'f');
    assert(status);

    // create file under file
    status = fs_create("user1", "/dir/file/file2", 'f');
    assert(status);

    // not a file or directory
    status = fs_create("user1", "testing", 'g');
    assert(status);

    // not a file or directory
    status = fs_create("user1", "/testing", 'g');
    assert(status);

    status = fs_writeblock("user1", "/dir", 0, writedata);
    assert(status);

    status = fs_readblock("user1", "/dir", 0, readdata);
    assert(status);

    status = fs_writeblock("user1", "/dir/file", 0, writedata);
    assert(!status);

    status = fs_readblock("user2", "/dir/file", 0, readdata);
    assert(!status);
    cout << std::string_view(readdata, 512) << std::endl;

    // create new direntry block
    for (int i = 0; i < 8; i++)
    {
        char num_char[6];
        std::sprintf(num_char, "%d", i);
        char cat[20] = "/dir/file";
        status = fs_create("user1", strcat(cat, num_char), 'f');
        assert(!status);
    }
    // occupy more than FS_DISKSIZE blocks
    // for (int i = 0; i < 4097; i++)
    // {
    //     char num_char[6];
    //     std::sprintf(num_char, "%d", i);
    //     char cat[20] = "/dir/file";
    //     status = fs_create("user1", strcat(cat, num_char), 'f');
    // }
    return 0;
}
