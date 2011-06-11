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

    byte_traits::byte_sequence somedata(src_array, src_array+src_arraysize);

    // create Serialized data from byte vector
	SerializedData::ptr_t serdat(
		new SerializedData(DataOwnership(), somedata.begin(), src_arraysize)
	);
	TEST_ASSERT(serdat->size() == src_arraysize);
	
	// create SegmentationLayer from Serialized data
	SegmentationLayer segmlayer(static_cast<BasicMessageLayer::ptr_t>(serdat));
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
        SegmentationLayer::HeaderType header =
            SegmentationLayer::decodeHeader(raw_ser.begin());
    }
    catch(const InvalidHeaderError&)
    { TEST_ASSERT(false && "Exception occured"); }

	BasicMessageLayer::dataptr_t body(
		new nuke_ms::byte_traits::byte_sequence(packetsize-4)
	);
	std::copy(it, raw_ser.end(), body->begin());
	
	SegmentationLayer rcvd(body);
	TEST_ASSERT(rcvd.size() == src_arraysize+4);
	
    return CONCLUDE_TEST();
}
