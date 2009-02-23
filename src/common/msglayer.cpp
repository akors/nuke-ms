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
#include "msglayer.hpp"

using namespace nms;

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
    // create new buffer, copy payload into it
    BasicMessageLayer::dataptr_type data(
        new byte_traits::byte_sequence(payload)
    );

    return data;
}




BasicMessageLayer::dataptr_type StringwrapLayer::serialize() const throw()
{
    // create new buffer, copy payload into it
    BasicMessageLayer::dataptr_type data(
        new byte_traits::byte_sequence(payload)
    );

    return data;
}

BasicMessageLayer::dataptr_type StringwrapLayer::getPayload() const throw()
{
    return this->serialize();
}



StringwrapLayer::StringwrapLayer(const std::wstring& msg) throw ()
    : payload(msg.length()*sizeof(std::wstring::value_type))
{
    // initialize payload to be as big as sizeof(std::wstring::value_type)
    // times the message length

    // one iterator for input, one for output
    byte_traits::byte_sequence::iterator out_iter = payload.begin();
    std::wstring::const_iterator in_iter = msg.begin();

    // write all bytes of one character into the buffer, advance the output
    // iterator
    for (; in_iter < msg.end(); in_iter++)
        out_iter = writebytes(out_iter, *in_iter);
}



std::wstring StringwrapLayer::getString() const throw()
{
    // assure that the payload length is a multiple of the
    // size of the character type (granted by the constructor)
    assert(!(payload.size() % sizeof(std::wstring::value_type)));

    // creat a string, initialize with the right size
    // and create iterator pointing at it
    std::wstring str((payload.size()/sizeof(std::wstring::value_type)), 'a');
    std::wstring::iterator out_iter = str.begin();

    // create a temporary value to store one wide character
    std::wstring::value_type tmpval;

    // pointer to the temporary value where the bytes will be written
    byte_traits::byte_t* tmpval_ptr;

    // iterate through all bytes in the sequence
    for (byte_traits::byte_sequence::const_iterator in_iter = payload.begin();
        in_iter < payload.end();)
    {
        // fill up the temporary character.
        // when the character is full, write it to the string.
        // move the byte pointer forward in each iteration
        for (tmpval_ptr = reinterpret_cast<byte_traits::byte_t*>(&tmpval);
            tmpval_ptr < reinterpret_cast<byte_traits::byte_t*>(&tmpval) + sizeof(tmpval);
            tmpval_ptr++, in_iter++)
        {
            *tmpval_ptr = *in_iter;
        }

        *out_iter++ = tmpval;
    }

    return str;
}


