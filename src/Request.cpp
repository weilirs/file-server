#include "Request.h"
#include <sstream>
#include <stdexcept>
#include <charconv>

ViewTokenizer::ViewTokenizer(const std::string_view &_source, const std::string_view &_delimiters, size_t _head) : source(_source),
                                                                                                                   delimiters(_delimiters), head(_head)
{
}

std::string_view ViewTokenizer::next(void)
{
    std::string_view token;
    size_t end_index = source.find_first_of(delimiters, head);
    if (end_index == std::string_view::npos)
    {
        token = source.substr(head);
        head = source.length();
    }
    else
    {
        token = source.substr(head, end_index - head);
        head = ++end_index;
    }
    return token;
}

bool ViewTokenizer::empty(void) const
{
    return head >= source.length();
}

ViewTokenizer::operator bool(void) const
{
    return !empty();
}

ViewTokenizer::~ViewTokenizer()
{
}

bool Request::parseOwner(const std::string_view &source, std::string_view &owner)
{
    bool valid = !source.empty() && source.length() <= FS_MAXUSERNAME;
    if (!valid)
    {
        throw std::runtime_error("owner parse failed");
    }
    owner = source;
    return valid;
}

bool Request::parsePath(const std::string_view &source, std::string_view &path)
{
    bool valid = !source.empty() && source.length() <= FS_MAXPATHNAME && source.front() == '/' && source.back() != '/';
    if (!valid)
    {
        throw std::runtime_error("path parse failed");
    }
    path = source;
    return valid;
}

bool Request::parsePathToken(const std::string_view &source, std::string_view &pathToken)
{
    bool valid = !source.empty() && source.length() <= FS_MAXFILENAME;
    if (!valid)
    {
        throw std::runtime_error("path token parse failed");
    }
    pathToken = source;
    return valid;
}

bool Request::parseBlock(const std::string_view &source, uint32_t &block)
{
    uint32_t result = 0;
    auto [_, ec]{std::from_chars(source.data(), source.data() + source.length(), result)};
    if (ec != std::errc())
    {
        throw std::runtime_error("numeric conversion failed in block parse");
    }
    else if ((source.front() == '0' && result) || result >= FS_MAXFILEBLOCKS)
    {
        throw std::runtime_error("block parse failed");
    }
    block = result;
    return true;
}

bool Request::parseType(const std::string_view &source, char &type)
{
    bool valid = source.length() == 1 && (source[0] == 'f' || source[0] == 'd');
    if (!valid)
    {
        throw std::runtime_error("type parse failed");
    }
    type = source[0];
    return valid;
}

bool Request::parseCommand(const std::string_view &source, CommandType &command)
{
    uint8_t result = 0;
    for (; result < NUM_COMMANDS && source != COMMANDS[result]; result++)
        ;
    if (result >= NUM_COMMANDS)
    {
        throw std::runtime_error("command parse failed");
    }
    command = (CommandType)(result);
    return true;
}

Request::Request() : owner(parseOwner), path(parsePath), pathTokens(), block(parseBlock), type(parseType),
                     command(parseCommand)
{
}

void Request::tokenizePath(void)
{
    ViewTokenizer tokenizer(path, "/", 1);
    while (tokenizer)
    {
        std::string_view token = tokenizer.next();
        pathTokens.emplace_back(token);
    }
}

void Request::parse(const std::string_view &request_str)
{
    message = request_str;
    std::string_view request_message(message);
    ViewTokenizer tokenizer(request_message, " ");
    command = tokenizer.next();
    owner = tokenizer.next();
    path = tokenizer.next();
    switch (command)
    {
    case FS_READBLOCK:
    case FS_WRITEBLOCK:
        block = tokenizer.next();
        break;
    case FS_CREATE:
        type = tokenizer.next();
        break;
    case FS_DELETE:
    default:
        break;
    }
    if (tokenizer)
    {
        throw std::runtime_error("incomplete tokenization");
    }
    tokenizePath();
}

std::string_view Request::messageView(void) const
{
    return std::string_view(message.c_str(), message.length());
}

bool PathTokenParser::operator()(const std::string_view &source, std::string_view &pathToken) const
{
    return Request::parsePathToken(source, pathToken);
}