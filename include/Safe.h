#pragma once

#include <functional>
#include <stdexcept>
#include <string_view>

template <typename T>
using Parser = std::function<bool(const std::string_view &, T &)>;

template <typename T, typename _Parser = Parser<T>>
class SafeParsed
{
protected:
    T value;
    _Parser parser;

public:
    SafeParsed() : value(), parser()
    {
        static_assert(!std::is_same<_Parser, Parser<T>>::value);
    }

    SafeParsed(const std::string_view &source) : value(), parser()
    {
        static_assert(!std::is_same<_Parser, Parser<T>>::value);
        parse(source);
    }

    SafeParsed(const _Parser &_parser) : value(), parser(_parser)
    {
    }

    SafeParsed(const std::string_view &source, const _Parser &_parser) : value(), parser(_parser)
    {
        parse(source);
    }

    const T &operator=(const std::string_view &source)
    {
        parse(source);
        return get();
    }

    bool parse(const std::string_view &source)
    {
        return parser(source, value);
    }

    const T &get(void) const
    {
        return value;
    }

    operator T const &(void) const
    {
        return get();
    }
};