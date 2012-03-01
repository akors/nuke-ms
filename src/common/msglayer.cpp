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

namespace nuke_ms {

// explicit class template instantions
template class BasicMessageLayer<SerializedData>;
template class BasicMessageLayer<StringwrapLayer>;
template class SegmentationLayer<SerializedData>;
template class SegmentationLayer<StringwrapLayer>;

// explicit function template instantions
template
byte_traits::byte_sequence::iterator StringwrapLayer::fillSerialized(
    byte_traits::byte_sequence::iterator it) const;
template SegmentationLayerBase::HeaderType SegmentationLayerBase::decodeHeader(
    byte_traits::byte_sequence::iterator headerbuf);

} // namespace nuke_ms


////////////////////////////// StringwrapLayer /////////////////////////////////


SerializedData& SerializedData::operator= (const SerializedData& other)
{
    if (&other == this) return *this;

    // create and copy memory block
    auto data = std::make_shared<byte_traits::byte_sequence>(_datasize);
    std::copy(other._begin_it, other._begin_it+other._datasize, data->begin());

    // assign ownership and iterator
    _memblock = data;
    _begin_it = data->begin();
    _datasize = other._datasize;

    return *this;
}

StringwrapLayer::StringwrapLayer(const SerializedData& msg)
{
    std::size_t datasize = msg.size();
    SerializedData::const_data_it data_it = msg.begin();

    // bail out if the string is not aligned
    if (datasize % sizeof(byte_traits::msg_string::value_type) !=0)
        throw MsgLayerError{"Unaligned packet"};

    // set message_string to the proper size
    _message_string.resize((datasize)/
        sizeof(byte_traits::msg_string::value_type));

    // iterator to the message_string
    byte_traits::msg_string::iterator out_iter = _message_string.begin();

    std::remove_reference<decltype(*out_iter)>::type tmpval;

    // iterate through all bytes in the sequence
    for (auto it = data_it; it < data_it + datasize; )
    {
        // read bytes into a character, convert byte endianness
        it = readbytes(&tmpval, it);
        *out_iter++ = to_hostbo(tmpval);
    }

}

