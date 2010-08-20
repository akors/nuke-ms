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


struct NearUserMessage : public ContainingLayer
{
    typedef boost::shared_ptr<NearUserMessage> ptr_t;
    typedef boost::shared_ptr<NearUserMessage> const_ptr_t;

    UniqueUserID recipient;
    UniqueUserID sender;

    /** Construct from a stringwraplayer message */
	NearUserMessage(
        StringwrapLayer::ptr_t stringwrap,
        const UniqueUserID& to = UniqueUserID(),
        const UniqueUserID& from = UniqueUserID()
    )
        : ContainingLayer(stringwrap), recipient(to), sender(from)
    { }

    virtual std::size_t size() const
    {
        return upper_layer->size() +
            sizeof(msg_id_t) + UniqueUserID::id_length*2;
    }

    virtual data_it fillSerialized(data_it buffer) const
    { return buffer; }
};

}

#endif // ifndef NEARTYPES_HPP_INCLUDED
