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


const UniqueUserID UniqueUserID::user_id_none;


NearUserMessage::NearUserMessage(const SerializedData& data)
    : ContainingLayer(BasicMessageLayer::ptr_t())
{
    BasicMessageLayer::const_data_it in_it = data.getDataIterator();

    // bail out, if data is too small
    if (data.size() < header_length)
        throw UndersizedPacketError();

    // if first byte isn't the correct layer identifier, that's a wrong packet
    if (*in_it++ != static_cast<byte_traits::byte_t>(LAYER_ID))
        throw InvalidHeaderError();


    // get msg id
    in_it = readbytes<byte_traits::uint4b_t>(&msg_id, in_it);

    // reverse msg id correctly
    msg_id = to_hostbo(msg_id);

    // recipient
    std::copy(in_it, in_it+UniqueUserID::id_length, recipient.id);
    in_it += UniqueUserID::id_length;

    // sender
    std::copy(in_it, in_it+UniqueUserID::id_length, sender.id);
    in_it += UniqueUserID::id_length;

    // the rest is the message string
    upper_layer = StringwrapLayer::ptr_t(
        new StringwrapLayer(SerializedData(
            data.getOwnership(), in_it, data.size()-header_length))
    );
}


std::size_t NearUserMessage::size() const
{
    return header_length + upper_layer->size();
}



BasicMessageLayer::data_it
NearUserMessage::fillSerialized(BasicMessageLayer::data_it buffer) const
{
    // first byte is layer identifier
    *buffer++ = static_cast<byte_traits::byte_t>(LAYER_ID);

    // next four bytes are the message id
    buffer = writebytes(
        buffer,
        to_netbo(msg_id)
    );

    // recipient
    buffer =
        std::copy(recipient.id, recipient.id+UniqueUserID::id_length, buffer);

    // sender
    buffer = std::copy(sender.id, sender.id+UniqueUserID::id_length, buffer);

    // the rest is the message string
    return upper_layer->fillSerialized(buffer);
}
