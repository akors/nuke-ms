#include "byteprinter.hpp"

std::string escapechar(char ch)
{
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
            retstring<<std::hex<<std::right<<std::setfill('0')<<
                static_cast<unsigned>(ch);
    }

    return retstring.str();
}

