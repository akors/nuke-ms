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


#ifndef NEARTYPES_HPP_INCLUDED
#define NEARTYPES_HPP_INCLUDED


#include <algorithm>
#include "bytes.hpp"
#include "msglayer.hpp"



namespace nuke_ms
{

typedef byte_traits::uint4b_t msg_id_t;

/** Class that identifies a user uniquely.
 * @note Although this class provides a similar interface to the MessageLayer
 * classes (such as the size() or fillSerialized() functions), it is NOT derived
 * from the BasicMessageLayer class.
*/
struct UniqueUserID
{
    enum { id_length = 8 /**< Length of a User ID */ };

    /** The User ID, as a series of bytes */
    byte_traits::byte_t id[id_length];

    /** Default constructor, does not initialize the User ID */
    UniqueUserID() {}

    /** Copy constructor */
    UniqueUserID(const UniqueUserID& other)
    { std::copy(other.id, other.id+id_length, this->id); }

    /** Construct ID from a buffer
     * @tparam RandomAccessIterator Random access output iterator type
     * @param in input buffer
    */
    template<typename RandomAccessIterator>
    UniqueUserID(RandomAccessIterator in)
    { std::copy(in, in+id_length, id); }


    /** Compare two ID's */
    bool operator == (const UniqueUserID& other) const
    { return std::equal(this->id, this->id+id_length, other.id); }

    /** Return serialized size of the User ID */
    inline std::size_t size() const
    { return id_length; }

    /** Write the ID into a buffer */
    template<typename OutputIterator>
    OutputIterator fillSerialized(OutputIterator buffer) const
    { return std::copy(id, id+id_length, buffer); }
};


/** Class representing a user message
 *
 * This class shall be used, whenever a client sends a message to another client
 * that is connected to the same server.
*/
class NearUserMessage : public BasicMessageLayer
{
    /** The string contained in this message */
    StringwrapLayer::ptr_t upper_layer;
public:
    typedef boost::shared_ptr<NearUserMessage> ptr_t;
    typedef boost::shared_ptr<NearUserMessage> const_ptr_t;

    enum { LAYER_ID = 0x41  /**< Layer Identifier */ };
    enum { header_length =
        1 + sizeof(msg_id_t) + UniqueUserID::id_length + UniqueUserID::id_length
    };

    /** ID of the message.
     * This object can be used to identify the message uniquely. This is
     * necessary for example when processing send reports
    */
    msg_id_t msg_id;

    /** Who this message is intended to.
     * Set this field to specify a recipient of the message
    */
    UniqueUserID recipient;

    /** Who sent this message
     * This field will be set to the client that sent the message.
     * It is not neccessary to specify this field when sending a message,
     * because it will be set by the protocol implicitly.
    */
    UniqueUserID sender;

    /** Construct from a stringwraplayer message
     * @param stringwrap The message to be sent
     * @param to Recipient of the message
     * @param from sender of the message
    */
	NearUserMessage(
        StringwrapLayer::ptr_t stringwrap,
        const UniqueUserID& to = UniqueUserID(),
        const UniqueUserID& from = UniqueUserID(),
        msg_id_t _msg_id = msg_id_t()
    )
        : upper_layer(stringwrap), recipient(to), sender(from),
            msg_id(_msg_id)
    {}

    /** Construct from a string
     * @param msg The message to be sent
     * @param to Recipient of the message
     * @param from sender of the message
    */
    NearUserMessage(
        const byte_traits::msg_string& msg,
        const UniqueUserID& to = UniqueUserID(),
        const UniqueUserID& from = UniqueUserID(),
        msg_id_t _msg_id = msg_id_t()
    )
        : upper_layer(StringwrapLayer::ptr_t(new StringwrapLayer(msg))),
            recipient(to), sender(from), msg_id(_msg_id)
    {}

    /** Construct from serialized Data
     *
     * @param data Serialized Data layer
     * @throws InvalidHeaderError when the stringwrap message is not aligned
     * @throws UndersizedPacketError when the datasize is less than the minimum
     * packet header
    */
    NearUserMessage(const SerializedData& data);

    // implementing base class version
    virtual std::size_t size() const;

    // implementing base class version
    virtual data_it fillSerialized(data_it buffer) const;

    /** Return the string contained in the layer message.
    *
    * This function merely redirects the call to the StringwrapLayer object.
    * @see StringwrapLayer::getString()
    *
    * @returns A constant reference to the string contained in this message
    */
    const byte_traits::msg_string& getString() const
    {
        return upper_layer->getString();
    }
};

}

#endif // ifndef NEARTYPES_HPP_INCLUDED
