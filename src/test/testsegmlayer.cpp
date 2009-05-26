#include <iostream>


#include "msglayer.hpp"
#include "byteprinter.hpp"

using nuke_ms::byte_traits;

int main()
{

    byte_traits::string s(L"This is a wide char string");
    std::cout<<"Original string (length "<<s.length()<<"): \n"<<
        printbytes(s.begin(), s.end())<<'\n';


    // first down the pipeline

    // create stringwrap from string
    nuke_ms::StringwrapLayer::ptr_type
    stringwrap_down(new nuke_ms::StringwrapLayer(s));

    // check string length
    std::cout<<"stringwrap_down->getSerializedSize() == "<<
        stringwrap_down->getSerializedSize()<<"\n";

    // create SegmentationLayer object from stringwrap
    nuke_ms::SegmentationLayer segmlayer(stringwrap_down);

    nuke_ms::BasicMessageLayer::dataptr_type bytewise(
        new nuke_ms::byte_traits::byte_sequence(
            segmlayer.getSerializedSize()
        )
    );

    segmlayer.fillSerialized(bytewise->begin());

    std::cout<<"serialized segmlayer (size "<<bytewise->size()<<"):\n"<<
        hexprint(bytewise->begin(), bytewise->end())<<'\n';

    return 0;
}