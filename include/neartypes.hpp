// neartypes.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2010, 2011  Alexander Korsunsky
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

/** @file neartypes.hpp
* @ingroup common
* @brief Types used for client-server communication
*
*/

#ifndef NEARTYPES_HPP_INCLUDED
#define NEARTYPES_HPP_INCLUDED


#include <iostream>
#include <algorithm>
#include <cstring>

#include "bytes.hpp"
#include "msglayer.hpp"



namespace nuke_ms
{

/** @addtogroup common
 * @{
*/

/** Class that identifies a user uniquely.
 * @note Although this class provides a similar interface to the MessageLayer
 * classes (such as the size() or fillSerialized() functions), it is NOT derived
 * from the BasicMessageLayer class.
*/
struct UniqueUserID
{
    /** The User ID, as a series of bytes */
    unsigned long long id;

    /** Length of a User ID */
    static constexpr std::size_t id_length = sizeof(id);

    /** UniqueUserID that means "no user id" */
    static const UniqueUserID user_id_none;

    /** Construct from long long uint.
    * _id Identifier as integer variable
    */
    UniqueUserID(unsigned long long _id = 0ull) : id(_id) {}

    /** Copy constructor */
    UniqueUserID(const UniqueUserID& other) = default;


    /** Construct ID from a buffer
     * @tparam RandomAccessIterator Random access output iterator type
     * @param in input buffer
    */
    template<typename RandomAccessIterator>
    UniqueUserID(RandomAccessIterator in)
    {
        std::copy(in,in+id_length, reinterpret_cast<byte_traits::byte_t*>(&id));
        id = to_hostbo(id);
    }

    /** Compare two ID's */
    bool operator == (const UniqueUserID& other) const
    { return this->id == other.id; }

    /** Return serialized size of the User ID */
    inline std::size_t size() const
    { return id_length; }

    /** Write the ID into a buffer */
    template<typename OutputIterator>
    OutputIterator fillSerialized(OutputIterator buffer) const
    { return writebytes(buffer, to_netbo(id)); }
};

/** Class representing a user message
 *
 * This class shall be used, whenever a client sends a message to another client
 * that is connected to the same server.
*/
struct NearUserMessage : BasicMessageLayer<NearUserMessage>
{
    /** Type for a more or less unique message identifier */
    typedef byte_traits::uint4b_t msg_id_t;

    /**< Layer Identifier */
    static constexpr byte_traits::byte_t LAYER_ID = 0x41;
    static constexpr std::size_t header_length =
        1+ sizeof(msg_id_t) + UniqueUserID::id_length + UniqueUserID::id_length;

    /** ID of the message.
     * This object can be used to identify the message uniquely. This is
     * necessary for example when processing send reports
    */
    msg_id_t _msg_id;

    /** Who this message is intended to.
     * Set this field to specify a recipient of the message
    */
    UniqueUserID _recipient;

    /** Who sent this message
     * This field will be set to the client that sent the message.
     * It is not neccessary to specify this field when sending a message,
     * because it will be set by the clientnode implicitly.
    */
    UniqueUserID _sender;

    StringwrapLayer _stringwrap;

    /** Construct from a stringwraplayer message
     * @param stringwrap The message to be sent
     * @param to Recipient of the message
     * @param from sender of the message
	 * @param _msg_id use this as message identifier
    */
	NearUserMessage(
        const StringwrapLayer& stringwrap,
        const UniqueUserID& to = UniqueUserID(),
        const UniqueUserID& from = UniqueUserID(),
        msg_id_t msg_id = msg_id_t()
    )
        : _stringwrap(stringwrap), _recipient(to), _sender(from),
            _msg_id(msg_id)
    { }

    /** Move-construct from a stringwraplayer message
     * @param stringwrap The message to be sent
     * @param to Recipient of the message
     * @param from sender of the message
	 * @param _msg_id use this as message identifier
    */
	NearUserMessage(
        StringwrapLayer&& stringwrap,
        const UniqueUserID& to = UniqueUserID(),
        const UniqueUserID& from = UniqueUserID(),
        msg_id_t msg_id = msg_id_t()
    )
        : _stringwrap(std::move(stringwrap)), _recipient(to), _sender(from),
            _msg_id(msg_id)
    { }

    /** Construct from serialized Data
     *
     * @param data Serialized Data layer
     *
     * @throw UndersizedPacketError when the datasize is less than the minimum
     * packet header
     * @throw InvalidHeaderError if the first byte of the data does not contain
     * the correct layer identifier.
    */
    NearUserMessage(const SerializedData& data);

    // implementing base class version
    std::size_t size() const
    { return header_length + _stringwrap.size(); }

    // implementing base class version
    template <typename ByteOutputIterator>
    ByteOutputIterator fillSerialized(ByteOutputIterator it) const;
};


template <typename ByteOutputIterator>
ByteOutputIterator NearUserMessage::fillSerialized(ByteOutputIterator it) const
{
    // first byte is layer identifier
    *it++ = static_cast<byte_traits::byte_t>(LAYER_ID);

    // next four bytes are the message id
    it = writebytes(
        it,
        to_netbo(_msg_id)
    );

    // recipient
    it = _recipient.fillSerialized(it);

    // sender
    it = _sender.fillSerialized(it);

    // the rest is the message string
    return _stringwrap.fillSerialized(it);
}


/**@}*/ // addtogroup common

extern template class BasicMessageLayer<NearUserMessage>;
extern template class SegmentationLayer<NearUserMessage>;

extern template byte_traits::byte_sequence::iterator
NearUserMessage::fillSerialized(byte_traits::byte_sequence::iterator it) const;


} // namespace nuke_ms

#endif // ifndef NEARTYPES_HPP_INCLUDED

