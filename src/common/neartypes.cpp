// neartypes.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2010  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "neartypes.hpp"

using namespace nuke_ms;

const UniqueUserID UniqueUserID::user_id_none = UniqueUserID();

namespace nuke_ms {

// explicit class template instantions
template class BasicMessageLayer<NearUserMessage>;
template class SegmentationLayer<NearUserMessage>;

// template function specializations
template byte_traits::byte_sequence::iterator
NearUserMessage::fillSerialized(byte_traits::byte_sequence::iterator it) const;

} // namespace nuke_ms

NearUserMessage::NearUserMessage(const SerializedData& data)
{
    auto in_it = data.begin();

    // bail out, if data is too small
    if (data.size() < header_length)
        throw UndersizedPacketError();

    // if first byte isn't the correct layer identifier that's a wrong packet
    if (*in_it++ != LAYER_ID) throw InvalidHeaderError();

    // get msg id
    in_it = readbytes<byte_traits::uint4b_t>(&_msg_id, in_it);

    // reverse msg id correctly
    _msg_id = to_hostbo(_msg_id);

    // recipient
    _recipient = UniqueUserID(in_it);
    in_it += UniqueUserID::id_length;

    // sender
    _sender = UniqueUserID(in_it);
    in_it += UniqueUserID::id_length;

    // the rest is the message string
    _stringwrap = StringwrapLayer{
        SerializedData{data.getOwnership(), in_it, data.size() - header_length}
    };
}

