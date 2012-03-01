#ifndef BYTEPRINTER_HPP
#define BYTEPRINTER_HPP

#include <iostream>
#include <sstream>
#include <iomanip>

std::string escapechar(char _ch);

template <typename ByteSequenceIterator>
struct BytePrinter
{
    ByteSequenceIterator begin, end;

    BytePrinter(ByteSequenceIterator _begin, ByteSequenceIterator _end)
        : begin{_begin}, end{_end}
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
    for (auto begin = p.begin; begin < p.end; ++begin)
    {
        if (isprint(*begin))
            os<<static_cast<char>(*begin);
        else
            //os<<escapechar(static_cast<char>(*begin));
            os<<'_';
    }
    return os;
}


template <typename ByteSequenceIterator>
struct HexPrinter
{
    ByteSequenceIterator begin, end;

    HexPrinter(ByteSequenceIterator _begin, ByteSequenceIterator _end)
        : begin{_begin}, end{_end}
    {}
};

template <typename ByteSequenceIterator>
HexPrinter<ByteSequenceIterator>
hexprint(ByteSequenceIterator begin, ByteSequenceIterator end)
{
    return HexPrinter<ByteSequenceIterator>{begin, end};
}

template <typename ByteSequenceIterator>
std::ostream& operator << (std::ostream& os,
    const HexPrinter<ByteSequenceIterator>& p)
{
    ByteSequenceIterator begin = p.begin;
    std::ios_base::fmtflags oldfmtflags = os.flags();

    for (; begin < p.end; ++begin)
    {
        os.width(2);
        os<<std::hex<<std::right<<
            std::setfill('0')<<static_cast<unsigned>(*begin)<<' ';
    }

    os.flags(oldfmtflags);
    return os;
}

#endif // ifndef BYTEPRINTER_HPP
