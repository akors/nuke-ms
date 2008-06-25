#ifndef MSGLAYERS_HPP_INCLUDED
#define MSGLAYERS_HPP_INCLUDED



#include <stdexcept>
#include <algorithm> // for min

#include "bytes.hpp"

namespace nms
{

enum LayerIds 
{
	SEGMENTATION_LAYER_ID = 0x80
};

struct MessageLayerError : public std::runtime_error
{
	MessageLayerError() throw()
		: std::runtime_error("An unknown Message Layer Error occured.\n")
	{}
	
	MessageLayerError(const std::string& msg) throw()
		: std::runtime_error(msg)
	{}
	
	virtual const char* what() throw()
	{ return message.c_str(); }
	
	virtual ~MessageLayerError() throw()
	{}

private:
	std::string message;
};


template <typename DerivedT>
class BasicMessageLayer
{
public:		
	bytes_sequence serialize()
	{
		return static_cast<DerivedT*>(this)->serialize();
	}
};


class SegmentationLayer : BasicMessageLayer<SegmentationLayer>
{
	bytes_sequence data;
	
public:
	enum { header_length = 4 };
	
	SegmentationLayer(const bytes_sequence& _data) throw (MessageLayerError)
	{ 
		// check for a valid header
		if ( (_data.size() < header_length) || 
				(_data[0] != static_cast<byte_t>(SEGMENTATION_LAYER_ID)) )
			throw MessageLayerError("Failed to construct Segmentation Layer Message");
		
		data.assign(_data.begin()+header_length, _data.end());
	}
	
	template <typename LayerT>
	SegmentationLayer(const BasicMessageLayer<LayerT>& msg)
		: data(msg.serialize())
	{}
	
	bytes_sequence serialize() const
	{ 
        // the serialized size is the size of the date plus the header length,
        // but at most as many bytes as a uint2b_t can describe
		size_t size = std::min<size_t>(data.size() + header_length, 
                                        std::numeric_limits<uint2b_t>::max());	
		
        // create empty sequence with the appropriate size
		bytes_sequence serialized(size);
		
		// first byte is the Layer Identifier
		serialized[0] = SEGMENTATION_LAYER_ID;
		
		// second and third bytes are the size of the whole packet
		uint2b_t sizebytes = hton2b(static_cast<uint2b_t>(size));		
		serialized[1] = *(reinterpret_cast<byte_t*>(&sizebytes));
		serialized[2] = *(reinterpret_cast<byte_t*>(&sizebytes)+1);
		
		// fourth byte is padded with zeros
		serialized[3] = 0;
		
		// the rest comes from the data portion
		std::copy(data.begin(), data.end(), serialized.begin()+4);
		
		return serialized; 		
	}	
	
};

};
#endif // ifndef MSGLAYERS_HPP_INCLUDED
