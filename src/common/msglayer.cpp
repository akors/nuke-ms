// msglayer.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2010  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>

#include "bytes.hpp"
#include "msglayer.hpp"

using namespace nuke_ms;




///////////////////////////// BasicMessageLayer ////////////////////////////////

SerializedData BasicMessageLayer::getSerializedData() const
    throw()
{
    std::size_t serializedsize = this->getSerializedSize();

    // allocate buffer with appropriate size
    dataptr_t data(new byte_traits::byte_sequence(serializedsize));

    // write serialized version of this layer into the buffer
    this->fillSerialized(data->begin());

    return SerializedData(data, data->begin(),serializedsize);
}




/////////////////////////////// SerializedData /////////////////////////////////


std::size_t SerializedData::getSerializedSize() const throw()
{
    // return the size of the memory block
    return datasize;
}


BasicMessageLayer::data_it
SerializedData::fillSerialized(data_it buffer) const
    throw()
{
    // copy the maintained data into the specified buffer
    return std::copy(
        begin_it,
        begin_it + datasize,
        buffer
    );
}


SerializedData SerializedData::getSerializedData() const
    throw()
{
    // this is a no-op, just copy the iterators and memory block
    return *this;
}


//////////////////////////// SegmentationLayer /////////////////////////////////

SegmentationLayer::SegmentationLayer(BasicMessageLayer::dataptr_t data)
    : serializedsize(data->size() + header_length),
      ContainingLayer(SerializedData::ptr_t(
            new SerializedData(
                data,
                data->begin(),
                data->size()
      )))
{
}

BasicMessageLayer::data_it
SegmentationLayer::fillSerialized(BasicMessageLayer::data_it buffer) const
    throw()
{
    // first byte is layer identifier
    *buffer++ = static_cast<byte_traits::byte_t>(LAYER_ID);

    // second and third bytes are the size of the whole packet
    buffer = writebytes(
        buffer,
        to_netbo(static_cast<byte_traits::uint2b_t>(serializedsize))
    );

    // fourth byte is a zero
    *buffer++ = 0;

    // the rest is the message
    return upper_layer->fillSerialized(buffer);
}



////////////////////////////// StringwrapLayer /////////////////////////////////

StringwrapLayer::StringwrapLayer(const SerializedData& msg)
    throw(MsgLayerError)
{
    std::size_t datasize = msg.getSerializedSize();
    const_data_it data_it = msg.getDataIterator();

    // bail out if the string is not aligned
    if (datasize % sizeof(byte_traits::string::value_type) !=0)
        throw MsgLayerError("Unaligned packet");

    // set message_string to the proper size
    message_string.resize((datasize)/sizeof(byte_traits::string::value_type));

    // iterator to the message_string
    byte_traits::string::iterator out_iter = message_string.begin();

    byte_traits::string::value_type tmpval;

    // iterate through all bytes in the sequence
    for ( const_data_it it = data_it; it < data_it + datasize; )
    {
        // read bytes into a character, convert byte endianness
        it = readbytes(&tmpval, it);
        *out_iter++ = to_hostbo(tmpval);
    }

}


std::size_t StringwrapLayer::getSerializedSize() const throw()
{
    return message_string.length() * sizeof(byte_traits::string::value_type);
}

BasicMessageLayer::data_it
StringwrapLayer::fillSerialized(BasicMessageLayer::data_it buffer) const
    throw()
{
    // an iterator to the message of string type
    byte_traits::string::const_iterator in_iter = message_string.begin();

    // write all bytes of one character into the buffer, advance the output
    // iterator
    for (; in_iter < message_string.end(); in_iter++)
        buffer = writebytes(buffer, to_netbo(*in_iter));

    return buffer;
}




