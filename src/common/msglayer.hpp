// msglayer.hpp

/*
 *   NMS - Nuclear Messaging System
 *   Copyright (C) 2008, 2009  Alexander Korsunsky
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




namespace nms
{


/** Class for errors with the message layers
* @ingroup proto
*/
class MsgLayerError : public std::runtime_error
{
    /** The error message */
    const char* msg;
public:
    /** Default Constructor */
    MsgLayerError() throw()
        : std::runtime_error("Unknown message layer error")
    { }

    /** Constructor.
    * @param str The error message
    */
    MsgLayerError(const char* str) throw()
        : std::runtime_error(str)
    {}

    /** Return error message as char array.
    * @return A null- terminated character array containg the error message
    */
    virtual const char* what() const throw()
    { return std::runtime_error::what(); }

    virtual ~MsgLayerError() throw()
    { }
};

struct InvalidHeaderError : public MsgLayerError
{
    InvalidHeaderError() throw()
        : MsgLayerError("Invalid packet header")
    {}
};


class BasicMessageLayer
{
public:
    typedef boost::shared_ptr<BasicMessageLayer> ptr_type;
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
     * Access shall be performed in ammortized  constant time.
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
    struct HeaderType {
        byte_traits::uint2b_t packetsize;
    };

    typedef boost::shared_ptr<SegmentationLayer> ptr_type;

    enum { LAYER_ID = 0x80 };
    enum { header_length = 4 };

    /** Construct from a message coming from the application. */
    SegmentationLayer(const BasicMessageLayer& upper_layer)
        throw (MsgLayerError)
    {
        // throw an error on oversized packets
        if (upper_layer.getSerializedSize() > getMaxDataLength())
            throw MsgLayerError("Oversized Packet");

        // serialize upper layer and store it in the payload
        payload = *upper_layer.serialize();
    }

    /** Construct from a message coming from the network. */
    SegmentationLayer(const byte_traits::byte_sequence& _payload)
        throw (MsgLayerError)
    {
        // throw an error on oversized packets
        if (_payload.size() > getMaxDataLength())
            throw MsgLayerError("Oversized Packet");

        // copy byte sequence into the local buffer
        payload = _payload;
    }

    template <typename InputIterator>
    static HeaderType decodeHeader(InputIterator headerbuf)
        throw(InvalidHeaderError);

    static std::size_t getMaxDataLength()
    {
        return std::numeric_limits<byte_traits::uint2b_t>::max() -header_length;
    }



    // overriden Methods of Base class
    virtual std::size_t getSerializedSize() const throw();
    virtual dataptr_type serialize() const throw ();
    virtual dataptr_type getPayload() const  throw();

};



template <typename InputIterator>
SegmentationLayer::HeaderType
SegmentationLayer::decodeHeader(InputIterator headerbuf)
    throw(InvalidHeaderError)
{
    HeaderType headerdata;

    // check first byte to be the correct layer identifier
    if ( headerbuf[0] != SegmentationLayer::LAYER_ID )
        throw InvalidHeaderError();

    // get the size of the packet

    *reinterpret_cast<byte_traits::byte_t*>(&headerdata.packetsize)
        = headerbuf[1];
    *(reinterpret_cast<byte_traits::byte_t*>(&headerdata.packetsize)+1)
        = headerbuf[2];

    headerdata.packetsize = ntohx(headerdata.packetsize);

    if (headerbuf[3] != 0)
        throw InvalidHeaderError();
}


/** Class that is used to wrap Strings into a Message Layer.
* This Class can be used to translate strings into byte sequences and vice
* versa.
*/
class StringwrapLayer : public BasicMessageLayer
{
    byte_traits::byte_sequence payload;

public:
    typedef boost::shared_ptr<StringwrapLayer> ptr_type;

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


} // namespace nms

#endif // ifndef MSGLAYERS_HPP_INCLUDED
