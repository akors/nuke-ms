#include <iostream>
#include <sstream>
#include <iomanip>

#include "bytes.hpp"
#include "msglayer.hpp"

using nuke_ms::byte_traits;

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

int main()
{
    byte_traits::string s(L"This is a wide char string");
    std::cout<<"Original string (length "<<s.length()<<"): \n"<<
        printbytes(s.begin(), s.end())<<'\n';


    // first down the pipeline

    // create stringwrap from string
    nuke_ms::StringwrapLayer stringwrap_down(s);

    // check string length
    std::cout<<"stringwrap_down.getSerializedSize() == "<<
        stringwrap_down.getSerializedSize()<<"\n";

    // serialize string
    nuke_ms::BasicMessageLayer::dataptr_type bytewise(
        new nuke_ms::byte_traits::byte_sequence(
            stringwrap_down.getSerializedSize()
        )
    );

    stringwrap_down.fillSerialized(bytewise->begin());

    std::cout<<"serialized stringwrap_down (size "<<bytewise->size()<<"): \n"<<
        printbytes(bytewise->begin(), bytewise->end())<<'\n';

    // create Unknown messagelayer from buffer
    nuke_ms::UnknownMessageLayer unknown(
        nuke_ms::DataOwnership(bytewise),
        bytewise->begin(),
        bytewise->size()
    );

    // create new stringwrap from unknown message layer
    nuke_ms::StringwrapLayer stringwrap_up(unknown);

    // get a string and print it
    std::cout<<"stringwrap_up.getString() == \n"<<
        std::string(
            stringwrap_up.getString().begin(),
            stringwrap_up.getString().end()
        )<<'\n';

    return 0;
}

std::string escapechar(char _ch)
{
    unsigned char ch = _ch;
    std::stringstream retstring;

    switch(ch)
    {
        case '\n':
            retstring<<"\\n";
            break;
        case '\t':
            retstring<<"\\t";
            break;
        case '\v':
            retstring<<"\\v";
            break;
        case '\b':
            retstring<<"\\b";
            break;
        case '\r':
            retstring<<"\\r";
            break;
        case '\f':
            retstring<<"\\f";
            break;
        case '\a':
            retstring<<"\\a";
            break;
        case '\\':
            retstring<<"\\\\";
            break;
        case '\'':
            retstring<<"\\'";
            break;
        case '\"':
            retstring<<"\\\"";
            break;

        default:
            retstring<<"\\x";
            retstring.width(2);
            retstring<<std::hex<<std::right<<std::setfill('0')<<static_cast<unsigned>(ch);
    }

    return retstring.str();
}

