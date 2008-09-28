// msglayer.cpp

/*
 *   NMS - Nuclear Messaging System
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


#include "bytes.hpp"
#include "protocol/msglayer.hpp"

using namespace nms;
using namespace protocol;

std::size_t SegmentationLayer::getSerializedSize() const throw()
{
    return header_length + payload.size();
}


BasicMessageLayer::dataptr_type SegmentationLayer::serialize() const throw()
{

    // create empty sequence with the appropriate size
    BasicMessageLayer::dataptr_type serialized(
        new byte_traits::byte_sequence(header_length + payload.size())
    );


    byte_traits::byte_sequence::iterator data_iterator = serialized->begin();

    // first byte is the Layer Identifier
    *data_iterator++ = static_cast<byte_traits::byte_t>(LAYER_ID);

    // second and third bytes are the size of the whole packet
    data_iterator = writebytes(
        data_iterator,
        htonx(static_cast<byte_traits::uint2b_t>(serialized->size()))
    );


    // fourth byte is padded with zeros
    *data_iterator++ = 0;

    // the rest comes from the data portion
    std::copy(payload.begin(), payload.end(), data_iterator);

    return serialized;
}


BasicMessageLayer::dataptr_type SegmentationLayer::getPayload() const throw()
{
    BasicMessageLayer::dataptr_type data(
        new byte_traits::byte_sequence(payload)
    );

    return data;
}


