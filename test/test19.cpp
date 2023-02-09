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

    const char writedata_manosk[512] = "# EECS 482 Teaching Schedule\n- Mon./Wed. 4:30 p.m. to 6:00 p.m.";
    const char writedata_barisk[512] = "# EECS 482 Teaching Schedule\n- Mon./Wed. 1:30 p.m. to 3:00 p.m.";
    char readdata[512];

    if (argc != 3)
    {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);

    status = fs_create("", "/home", 'd');
    assert(status);

    status = fs_create("root", "/home", 'd');
    assert(!status);

    status = fs_create("manosk", "/home/manosk", 'd');
    assert(status);

    status = fs_create("manosk", "/manosk", 'd');
    assert(!status);

    status = fs_create("barisk", "/barisk", 'd');
    assert(!status);

    status = fs_create("manosk", "/manosk/barisk", 'd');
    assert(!status);

    status = fs_create("barisk", "/barisk/manosk", 'd');
    assert(!status);

    status = fs_create("manosk", "/manosk/482_schedule.md", 'f');
    assert(!status);

    status = fs_create("barisk", "/barisk/482_schedule.md", 'f');
    assert(!status);

    status = fs_writeblock("manosk", "/manosk/482_schedule.md", 0, writedata_manosk);
    assert(!status);

    status = fs_writeblock("barisk", "/barisk/482_schedule.md", 0, writedata_barisk);
    assert(!status);

    status = fs_readblock("manosk", "/manosk/482_schedule.md", 0, readdata);
    assert(!status);
    std::cout << std::string_view(readdata, 512) << std::endl;

    status = fs_readblock("barisk", "/barisk/482_schedule.md", 0, readdata);
    assert(!status);
    std::cout << std::string_view(readdata, 512) << std::endl;

    status = fs_create("manosk", "/manosk/barisk/482_schedule.md", 'f');
    assert(!status);

    status = fs_create("barisk", "/barisk/manosk/482_schedule.md", 'f');
    assert(!status);

    status = fs_writeblock("manosk", "/manosk/barisk/482_schedule.md", 0, writedata_barisk);
    assert(!status);

    status = fs_writeblock("barisk", "/barisk/manosk/482_schedule.md", 0, writedata_manosk);
    assert(!status);

    status = fs_readblock("manosk", "/manosk/482_schedule.md", 0, readdata);
    assert(!status);
    std::cout << std::string_view(readdata, 512) << std::endl;

    status = fs_readblock("barisk", "/barisk/482_schedule.md", 0, readdata);
    assert(!status);
    std::cout << std::string_view(readdata, 512) << std::endl;

    status = fs_delete("manosk", "/manosk/barisk");
    assert(status);

    status = fs_delete("barisk", "/barisk/manosk");
    assert(status);

    status = fs_delete("manosk", "/manosk/barisk/482_schedule.md");
    assert(!status);

    status = fs_delete("barisk", "/barisk/manosk/482_schedule.md");
    assert(!status);

    status = fs_delete("manosk", "/manosk/barisk");
    assert(!status);

    status = fs_delete("barisk", "/barisk/manosk");
    assert(!status);

    status = fs_delete("manosk", "/manosk/482_schedule.md");
    assert(!status);

    status = fs_delete("barisk", "/barisk/482_schedule.md");
    assert(!status);

    status = fs_delete("manosk", "/manosk");
    assert(!status);

    status = fs_delete("barisk", "/barisk");
    assert(!status);

    return 0;
}