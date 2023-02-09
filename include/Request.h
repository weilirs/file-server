#pragma once

#include "fs_server.h"
#include "Safe.h"
#include <array>
#include <string>
#include <vector>

typedef enum __command_t : uint8_t
{
    FS_READBLOCK = 0,
    FS_WRITEBLOCK = 1,
    FS_CREATE,
    FS_DELETE
} CommandType;

constexpr size_t FS_MAXREQUESTMSG = 148;

class ViewTokenizer
{
protected:
    std::string_view source;
    std::string delimiters;
    size_t head;

public:
    ViewTokenizer(const std::string_view &_source, const std::string_view &_delimiters, size_t _head = 0);
    std::string_view next(void);
    bool empty(void) const;
    operator bool(void) const;
    ~ViewTokenizer();
};

class PathTokenParser;

class Request
{
public:
    static constexpr size_t MAX_COMMAND_LENGTH = 13;
    static constexpr size_t NUM_COMMANDS = 4;
    static constexpr char COMMANDS[NUM_COMMANDS][MAX_COMMAND_LENGTH + 1] = {"FS_READBLOCK", "FS_WRITEBLOCK", "FS_CREATE", "FS_DELETE"};

    using SafePathToken = SafeParsed<std::string_view, PathTokenParser>;

    SafeParsed<std::string_view> owner;
    SafeParsed<std::string_view> path;
    std::vector<SafePathToken> pathTokens;
    SafeParsed<uint32_t> block;
    SafeParsed<char> type;
    SafeParsed<CommandType> command;

protected:
    std::string message;
    void tokenizePath(void);

public:
    Request();
    void parse(const std::string_view &request_str);
    std::string_view messageView(void) const;
    static bool parseOwner(const std::string_view &source, std::string_view &owner);
    static bool parsePath(const std::string_view &source, std::string_view &path);
    static bool parsePathToken(const std::string_view &source, std::string_view &pathToken);
    static bool parseBlock(const std::string_view &source, uint32_t &block);
    static bool parseType(const std::string_view &source, char &type);
    static bool parseCommand(const std::string_view &source, CommandType &command);
};

class PathTokenParser
{
public:
    bool operator()(const std::string_view &source, std::string_view &pathToken) const;
};