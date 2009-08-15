#include <iostream>
#include <sstream>
#include <iomanip>

#include "byteprinter.hpp"
#include "bytes.hpp"
#include "msglayer.hpp"

using nuke_ms::byte_traits;


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
