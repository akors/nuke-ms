#include <iostream>


#include "msglayer.hpp"
#include "byteprinter.hpp"

using namespace nuke_ms;

int main()
{

    byte_traits::string s(L"This is a wide char string");
    std::cout<<"Original string (length "<<s.length()<<"): \n"<<
        printbytes(s.begin(), s.end())<<'\n';


    // first down the pipeline

    // create stringwrap from string
    StringwrapLayer::ptr_t stringwrap_down(new StringwrapLayer(s));

    // check string length
    std::cout<<"stringwrap_down->getSerializedSize() == "<<
        stringwrap_down->getSerializedSize()<<"\n";

    // create SegmentationLayer object from stringwrap
    // ugly static cast is needed, because otherwise call would be ambiguous
    SegmentationLayer
    segmlayer(static_cast<BasicMessageLayer::ptr_t>(stringwrap_down));

    BasicMessageLayer::dataptr_t bytewise(
        new nuke_ms::byte_traits::byte_sequence(
            segmlayer.getSerializedSize()
        )
    );

    segmlayer.fillSerialized(bytewise->begin());

    std::cout<<"serialized segmlayer (size "<<bytewise->size()<<"):\n"<<
        hexprint(bytewise->begin(), bytewise->end())<<'\n';


    // now up the pipeline


    // parse the header and check for errors
    try {
        SegmentationLayer::HeaderType header =
            SegmentationLayer::decodeHeader(bytewise->begin());
    }
    catch(const InvalidHeaderError&)
    { std::cout<<"Error reading packet header.\n"; }

    // construct segmentation layer from the unknonw message layer
    SegmentationLayer
    segmlayer_up((bytewise));

    // stringwrap from unknown layer
    try {
        StringwrapLayer s2(segmlayer_up.getUpperLayer()->getSerializedData());

        std::cout<<"The sent string has size "<<std::dec<<
            s2.getString().size()<<" and was: ";

        std::wcout<<s2.getString()<<std::endl;
    }
    catch(const MsgLayerError&)
    { std::cout<<"Error constructing stringwrap package\n"; }

    return 0;
}
