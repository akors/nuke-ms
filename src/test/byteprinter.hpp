#include <iostream>
#include <sstream>
#include <iomanip>

#ifndef BYTEPRINTER_HPP
#define BYTEPRINTER_HPP

std::string escapechar(char _ch);

template <typename ByteSequenceIterator>
struct BytePrinter
{
    ByteSequenceIterator begin, end;

    BytePrinter(ByteSequenceIterator _begin, ByteSequenceIterator _end)
        : begin(_begin), end(_end)
    {}
};

template <typename ByteSequenceIterator>
BytePrinter<ByteSequenceIterator>
printbytes(ByteSequenceIterator begin, ByteSequenceIterator end)
{
    return BytePrinter<ByteSequenceIterator>(begin, end);
}

template <typename ByteSequenceIterator>
std::ostream& operator << (std::ostream& os,
    const BytePrinter<ByteSequenceIterator>& p)
{
    ByteSequenceIterator begin = p.begin;

    for (; begin < p.end; ++begin)
    {
        if (isprint(*begin))
            os<<static_cast<char>(*begin);
        else
            //os<<escapechar(static_cast<char>(*begin));
            os<<'_';
    }
    return os;
}

#endif // ifndef BYTEPRINTER_HPP
