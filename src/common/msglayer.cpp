// msglayer.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
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


UnknownMessageLayer::UnknownMessageLayer(
    DataOwnership _memblock,
    BasicMessageLayer::const_data_iterator _data_it,
    std::size_t _datasize,
    bool new_memory_block
)
    : memblock(), data_it(_data_it), datasize(_datasize)
{
    // if we don't want a new memory block, everything is fine
    if (!new_memory_block)
    {
        memblock = _memblock;
        data_it = _data_it;
    }
    // if we do want one, we have to allocate a new buffer and copy the data
    else
    {
        // allocate buffer with appropriate size
        BasicMessageLayer::dataptr_type data(
            new byte_traits::byte_sequence(_datasize)
        );

        // copy buffer
        std::copy(
            _data_it,
            _data_it + _datasize,
            data->begin()
        );

        // assign ownership
        memblock = DataOwnership(data);

        // asign data iterator
        data_it = data->begin();
    }
}


std::size_t UnknownMessageLayer::getSerializedSize() const throw()
{
    // return the size of the memory block
    return datasize;
}


BasicMessageLayer::data_iterator
UnknownMessageLayer::fillSerialized(data_iterator buffer) const
    throw()
{
    // copy the maintained data into the specified buffer
    return std::copy(
        data_it,
        data_it + datasize,
        buffer
    );
}

StringwrapLayer::StringwrapLayer(const byte_traits::string& msg) throw ()
    : message_string(msg)
{}


StringwrapLayer::StringwrapLayer(const UnknownMessageLayer& msg)
    throw(MsgLayerError)
{
    std::size_t datasize = msg.getSerializedSize();
    const_data_iterator data_it = msg.getDataIterator();

    // bail out if the string is not aligned
    if (datasize % sizeof(byte_traits::string::value_type) !=0)
        throw MsgLayerError("Unaligned packet");

    // set message_string to the proper size
    message_string.resize((datasize)/sizeof(byte_traits::string::value_type));

    // iterator to the message_string
    byte_traits::string::iterator out_iter = message_string.begin();

    byte_traits::string::value_type tmpval;

    // iterate through all bytes in the sequence
    for ( const_data_iterator it = data_it; it < data_it + datasize; )
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

BasicMessageLayer::data_iterator
StringwrapLayer::fillSerialized(BasicMessageLayer::data_iterator buffer) const
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

SegmentationLayer::SegmentationLayer(BasicMessageLayer::dataptr_type data)
    : datasize(data->size())
{
    upper_layer = UnknownMessageLayer::ptr_type(
        new UnknownMessageLayer(DataOwnership(data), data->begin(), datasize)
    );
}

BasicMessageLayer::data_iterator
SegmentationLayer::fillSerialized(BasicMessageLayer::data_iterator buffer) const
    throw()
{
    // first byte is layer identifier
    *buffer++ = static_cast<byte_traits::byte_t>(LAYER_ID);

    // second and third bytes are the size of the whole packet
    buffer = writebytes(
        buffer,
        to_netbo(static_cast<byte_traits::uint2b_t>(datasize+header_length))
    );

    // fourth byte is a zero
    *buffer++ = 0;

    // the rest is the message
    return upper_layer->fillSerialized(buffer);
}
