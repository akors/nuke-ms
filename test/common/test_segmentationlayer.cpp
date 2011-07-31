#include <iostream>


#include "msglayer.hpp"
#include "neartypes.hpp"
#include "byteprinter.hpp"
#include "testutils.hpp"


DECLARE_TEST("class SegmentationLayer")


using namespace nuke_ms;

int main()
{
	int src_array[] = {11, 22, 33, 44, 0xaa, 0xbb, 0xcc, 0xdd};
	const int src_arraysize = sizeof(src_array)/sizeof(*src_array);

    byte_traits::byte_sequence somedata(src_array, src_array + src_arraysize);
    SerializedData serdat_down(DataOwnership(), somedata.begin(), src_arraysize);
	TEST_ASSERT(serdat_down.size() == src_arraysize);

	// create SegmentationLayer from Serialized data
	SegmentationLayer<SerializedData> segmlayer(std::move(serdat_down));
	TEST_ASSERT(segmlayer.size() == src_arraysize + 4);

	// serialize to buffer
	byte_traits::byte_sequence raw_ser(segmlayer.size());
	byte_traits::byte_sequence::iterator it = raw_ser.begin();
	segmlayer.fillSerialized(it);

	std::cout<<"Source array: "<<hexprint(src_array, src_array+src_arraysize)<<
	'\n'<<
	"Serialized Packet: "<<hexprint(raw_ser.begin(), raw_ser.end())<<'\n';

	// check correctness of message data
	TEST_ASSERT(*it++ == 0x80);
	byte_traits::uint2b_t packetsize;
	it = readbytes(&packetsize, it);
	TEST_ASSERT(to_hostbo(packetsize) == src_arraysize+4);
	TEST_ASSERT(*it++ == 0);
	TEST_ASSERT(std::equal(it, raw_ser.end(), &src_array[0]));

    try {
        // construct from "network data"
        SegmentationLayerBase::HeaderType header =
            SegmentationLayerBase::decodeHeader(raw_ser.begin());

        // create buffer for message data
        std::shared_ptr<byte_traits::byte_sequence> body(
            new byte_traits::byte_sequence(header.packetsize-4)
        );
        // "read" body from network
        std::copy(it, raw_ser.end(), body->begin());

        // pretty C++0x "uniform initialization" syntax
        SegmentationLayer<SerializedData> rcvd({body, body->begin(), body->size()});
        TEST_ASSERT(rcvd.size() == src_arraysize + 4);

        // retrieve data, steal from from rcvd object
        SerializedData serdat_up(std::move(rcvd._inner_layer));
        TEST_ASSERT(rcvd.size() == 4); // rcvd now has to be empty
        TEST_ASSERT( // data must of course be still the same
            std::equal(serdat_up.getDataIterator(),
                serdat_up.getDataIterator() + serdat_up.size(),
                &src_array[0])
        );

        std::cout<<"Data received: "<<hexprint(serdat_up.getDataIterator(),
                serdat_up.getDataIterator() + serdat_up.size())<<std::endl;
    }
    catch(const InvalidHeaderError&)
    { TEST_ASSERT(false && "Exception occured"); }



    return CONCLUDE_TEST();
}
