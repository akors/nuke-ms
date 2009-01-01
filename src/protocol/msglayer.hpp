// msglayer.hpp

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



#ifndef MSGLAYERS_HPP_INCLUDED
#define MSGLAYERS_HPP_INCLUDED



#include <stdexcept>
#include <limits>

#include <boost/shared_ptr.hpp>

#include "bytes.hpp"
#include "protocol/errors.hpp"


namespace nms
{
namespace protocol
{


class BasicMessageLayer
{
public:
    typedef boost::shared_ptr<byte_traits::byte_sequence> dataptr_type;

    /** Virtual destructor */
    virtual ~BasicMessageLayer()
    {}

    /** Retrieve payload.
     * Returns a pointer to the byte sequence holding the payload of
     * the packet.
     *
     * @return A pointer to the payload of the packet.
     */
    virtual dataptr_type getPayload() const  throw() = 0;


    /** Retrieve the serialized size.
     * Returns the length of the returned sequence of a successive call to
     * serialize(). <br>
     * Access shall be performed in constant ammortized time.
     *
     * @return The number of bytes the serialized byte sequence would have.
    */
    virtual std::size_t getSerializedSize() const throw() = 0;

    /** Retrieve serialized version of this Layer.
     *
     * Turns the current object into a series of bytes that can be sent onto
     * the wire.
     *
     * @return A pointer to the serialized data of this object.
     */
    virtual dataptr_type serialize() const throw () = 0;
};




class SegmentationLayer : public BasicMessageLayer
{
    byte_traits::byte_sequence payload;

public:
    enum { LAYER_ID = 0x80 };
    enum { header_length = 4 };

    /** Construct from a message coming from the application. */
    SegmentationLayer(const BasicMessageLayer& upper_layer)
        throw (ProtocolError)
    {
        // throw an error on oversized packets
        if (upper_layer.getSerializedSize() > getMaxDataLength())
            throw ProtocolError("Oversized Packet");

        // serialize upper layer and store it in the payload
        payload = *upper_layer.serialize();
    }

    /** Construct from a message coming from the network. */
    SegmentationLayer(const byte_traits::byte_sequence& _payload)
        throw (ProtocolError)
    {
        // throw an error on oversized packets
        if (_payload.size() > getMaxDataLength())
            throw ProtocolError("Oversized Packet");

        // copy byte sequence into the local buffer
        payload = _payload;
    }


    static std::size_t getMaxDataLength()
    {
        return std::numeric_limits<byte_traits::uint2b_t>::max() -header_length;
    }


    virtual std::size_t getSerializedSize() const throw();

    virtual dataptr_type serialize() const throw ();

    virtual dataptr_type getPayload() const  throw();
};


/** Class that is used to wrap Strings into a Message Layer.
* This Class can be used to translate strings into byte sequences and vice
* versa.
*/
class StringwrapLayer : public BasicMessageLayer
{
    byte_traits::byte_sequence payload;

public:

    /** Construct from a a std::wstring.
    * Of copies each byte of each character into the payload of the layer.
    * @param msg The msg that you want to wrap
    */
    StringwrapLayer(const std::wstring& msg) throw ();

    /** Construct from a message coming from the network. */
    StringwrapLayer(const byte_traits::byte_sequence& _payload)
        : payload(_payload)
    { }


    std::wstring getString() const throw();


    virtual std::size_t getSerializedSize() const throw()
    {
        return payload.size();
    }


    virtual BasicMessageLayer::dataptr_type serialize() const throw();

    virtual BasicMessageLayer::dataptr_type getPayload() const  throw();
};



} // namespace protocol
} // namespace nms

#endif // ifndef MSGLAYERS_HPP_INCLUDED
