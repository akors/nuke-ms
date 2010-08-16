#include <iostream>


#include "msglayer.hpp"
#include "byteprinter.hpp"

using namespace nuke_ms;

int main()
{
    byte_traits::msg_string s("This is a narrow char string");
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


    SerializedData serialized = segmlayer.getSerializedData();

    BasicMessageLayer::dataptr_t bytewise(
        new nuke_ms::byte_traits::byte_sequence(
            serialized.getSerializedSize()
        )
    );

    serialized.fillSerialized(bytewise->begin());

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

    // when getting messages from the network, we first parse the header and
    // then we get the rest of the message.
    BasicMessageLayer::dataptr_t rest_msg(
        new nuke_ms::byte_traits::byte_sequence(
            bytewise->begin()+SegmentationLayer::header_length, bytewise->end()
        )
    );

    // construct segmentation layer from the unknown message layer
    SegmentationLayer
    segmlayer_up(rest_msg);

    // stringwrap from unknown layer
    try {
        StringwrapLayer s2(segmlayer_up.getUpperLayer()->getSerializedData());

        std::cout<<"The sent string has size "<<std::dec<<
            s2.getString().size()<<" and was: \"";

        std::cout<<s2.getString()<<'\"'<<std::endl;
    }
    catch(const MsgLayerError&)
    { std::cout<<"Error constructing stringwrap package\n"; }

    return 0;
}
