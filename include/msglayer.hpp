// msglayer.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2009, 2010, 2011  Alexander Korsunsky
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

/** @file msglayer.hpp
* @ingroup common
* @brief Message Layer Interface
*
* This file contains declarations and definitions of the Message Layer
* infrastructure.
*
* When data is intended to be sent over the network, the data normally is
* encapsuled in many layers, several "headers" are attached to it.
*
* The idea behind this class hierarchy is to ease and unify the handling of
* encoding and decoding messages into messages of a lower or upper layers.
* To clarify the use of this class hierarchy, one hypothetical procedure of
* sending a message is explained.
*
* The message travels from the user "down the pipeline" to the network, where it
* can be sent.
* At the beginning the user enters some data (e.g. a text message). This data is
* encapsuled in a wrapper tagging the message with metadata (like username,
* font...). Let's call this layer "layer A".
* Then message of layer A is encrypted becomes a message of layer b.
* Finally the message is serialized. To ensure completeness of the message, it
* is encapsuled in a segmentation layer, layer C.
*
* At this moment the message looks like this:
* A layer C object holds a copy of the layer B object, which holds a copy of the
* layer A object, which holds a copy of the text the user entered.
* Now, the message can be sent over the network.
*
*
* This is what happens on the receiving site:
* The header of the segmentation layer message (layer C) arrives at the
* receiving site. The receiving site allocates memory and receives the
* rest of the message.
* Then, the message is passed "up the pipeline" to a responsible function
* which determines the correct layer of the message (layer B in our case) and
* passes the message on to the next layer handling function. This layer B
* function unpacks the encapsuled layer A message which is then finally
* displayed back to the user.
*
*
* Implementation
*
* The message pipeline is implemented by a class for each layer. An object of
* this class is therefore considered as a message of this layer.
* Each layer holds a copy of its upper layer.
*
* Normally a layer only adds a small header (compared to the size of the
* original message) to the message and passes it down to the next layer. It
* would be waste of memory and computation time to copy the data of each upper
* layer into the current layer.
* To avoid this, layers should make use of the RValue/move semantics provided
* by ISO C++11 and move (instead of copy) an upper layer object into place.
*
*
*   Sending messages
*
* When sending messages, the size of the message has to be determined
* and the data has to be serialized.
* To determine the size, a request is made up the pipeline:
* layer C asks layer B, which in turn asks A which knows
* the size of the original message. The value travels back down the pipeline,
* where at each step the header size of each layer is added.
* To serialize the message, layer C serializes its header and asks B to
* serialize. B in turn serializes its header and asks A to serialize. A
* serializes its header and appends the text data. The network layer can then
* send the serialized data.
*
*  Receiving messages
*
* The main memory allocation for an incoming message should be performed at the
* very beginning, right before receiving the message body of the segmentation
* layer. The whole block should be kept in memory and the message layers pass
* ownership to this block and iterators upwards. This is a tradeoff:
* At the expense of unnessecary bytes being held in memory, additional memory
* allocations and copy operations can be prevented. This should be a good
* tradeoff because "normally" the headers on the messages are relatively small
* and memory waste is neglectable.
* If for a layer the cost becomes to high (if its header is rather big), it
* can decide to discard the old buffer and allocate a new smaller buffer.
*
*
* @author Alexander Korsunsky
*
*/


#ifndef MSGLAYERS_HPP_INCLUDED
#define MSGLAYERS_HPP_INCLUDED

#include <stdexcept>
#include <limits>
#include <memory>
#include <type_traits>

#include "bytes.hpp"



namespace nuke_ms
{

/** @addtogroup common
 * @{
*/

/** Class for errors with the message layers
*/
class MsgLayerError : public std::runtime_error
{
    /** The error message */
    const char* msg;
public:
    /** Default Constructor */
    MsgLayerError() throw()
        : std::runtime_error{"Unknown message layer error"}
    { }

    /** Constructor.
    * @param str The error message
    */
    MsgLayerError(const char* str) throw()
        : std::runtime_error{str}
    {}

    /** Return error message as char array.
    * @return A null- terminated character array containg the error message
    */
    virtual const char* what() const throw()
    { return std::runtime_error::what(); }

    virtual ~MsgLayerError() throw()
    { }
};

/** Type to denote invalid header encoding */
struct InvalidHeaderError : public MsgLayerError
{
    InvalidHeaderError()
        : MsgLayerError{"Invalid packet header"}
    {}
};

/** Type to denote undersized packet */
struct UndersizedPacketError : public MsgLayerError
{
    UndersizedPacketError()
        : MsgLayerError{"Packet too small"}
    {}
};


/** Message Layer base class.
*
* This virtual base class presents a basic interface to it's deriving classes.
* Derived classes are said to represent "layers" of the communication pipeline,
* and objects of these deriving classes represent "messages" of their layer
* in the communication pipeline.
* <br>
* Derived classes should override the virtual functions according to the
* documentation.
* Also, they should at least provide a constructor that creates an instance of
* the derived class from serialized data. If this data is malformed, the
* constructor should throw an instance of MsgLayerError.
*/
template <typename DerivedType>
struct BasicMessageLayer
{

    /** Retrieve the serialized size.
     * Returns the length of the returned sequence of a successive call to
     * serialize().
     *
     * @return The number of bytes the serialized byte sequence would have.
    */
    std::size_t size() const
    { return static_cast<const DerivedType*>(this)->size(); }


    /** Fill a buffer with the serialized version of this object.
    * This function serializes this layer (and its upper layers) and writes
    * the bytes into a buffer that is pointed to by buffer.
    * The buffer has to have at least the size that is returned by the
    * size() function. Beware! No checks will be performed
    * to assure proper buffer size. Use the function size() to
    * obtain the minimal required size.
    *
    * @tparam ByteOutputIterator
    * @param buffer An iterator pointing to a range in the buffer that will be
    * filled
    *
    * @returns An iterator pointing past the filled range in the buffer. This
    * is the same iterator as buffer but it is incremented size()
    * times.
    */
    template <typename ByteOutputIterator>
    ByteOutputIterator fillSerialized(ByteOutputIterator it) const
    { return DerivedType::fillSerialized(it); }
};


/** Message of an unknown message layer.
 *
 * This class should be used to hold a serialized message of an unknown type.
 * It holds only the reference to the memory block and an iterator to the data.
 * It provides a possibility to access the contained data to determine the real
 * message layer class.
 *
 * To obtain ownership, use the getOwnership() member function,
 * to obtain an iterator to the data, use the begin() member function.
*/
class SerializedData : public BasicMessageLayer<SerializedData>
{
    /** Ownership of the memory block */
    std::shared_ptr<const byte_traits::byte_sequence> _memblock;

    /**< Iterater to the beginning data */
    decltype(_memblock->begin()) _begin_it;

    /**< Size of the data */
    std::size_t _datasize;

public:
    typedef byte_traits::byte_sequence::const_iterator const_data_it;

    /** Copy Constructor.
    * Constructs a new object from another object, by creating a new buffer and
    * copying the content.
    * The ownership of the data will be distinct and independent from the other
    * object.
    *
    * @param other Object to copy data from
    * @note Any data in the buffer before other.begin() will be discarded.
    */
    explicit SerializedData(const SerializedData& other)
    {
        *this = other; // use assignment operator
    }

    /** Copy assignment operator.
    * Constructs a new object from another object, by creating a new buffer and
    * copying the content.
    * The ownership of the data will be distinct and independent from the other
    * object.
    *
    * @param other Object to copy data from
    *
    * @note Any data in the buffer before other.begin() will be discarded.
    */
    SerializedData& operator= (const SerializedData& other);

    /** Move constructor
    * Constructs a new object from another object, taking over the buffer
    * ownership of the other object.
    *
    * @param other Object to obtain data from
    *
    * @note After invocation of this constructor, other will be invalidated and
    * should not be used.
    */
    SerializedData(SerializedData&& other) = default;

    /** Move assignment operator.
    * Constructs a new object from another object, taking over the buffer
    * ownership of the other object.
    *
    * @param other Object to obtain data from
    *
    * @note After invocation of this operator, other will be invalidated and
    * should not be used.
    */
    SerializedData& operator= (SerializedData&&) = default;

    /** Constructor.
    * Constructs a new object and passes memory ownership, data iterator and
    * data size. Note that the memory block can contain more data than is
    * accessible with the data iterator. To ensure the validity of _begin_it,
    * you have to pass a memory ownership object.
    *
    * @param _memblock Ownership of a memory block that ensures that _data_it is
    * valid
    * @param _begin_it Iterator to the data of the message in the memory block
    * block
    * @param _datasize size of the message in bytes
    */
    SerializedData(
        std::shared_ptr<const byte_traits::byte_sequence> memblock,
        const_data_it begin_it,
        std::size_t datasize
    )
        : _memblock{memblock}, _begin_it{begin_it}, _datasize{datasize}
    {}


    // overriding base class version
    std::size_t size() const
    { return _datasize; }

    // overriding base class version
    template <typename ByteOutputIterator>
    ByteOutputIterator fillSerialized(ByteOutputIterator it) const
    {
        // copy the maintained data into the specified buffer
        return std::copy(_begin_it, _begin_it + _datasize, it);
    }

    /** Get iterator to message data.
    * This function can be used to access the buffer directly, either to copy
    * the contained data into a buffer or to construct a message of an upper
    * layer.
    *
    * @returns An iterator to the message data. This iterator is valid as long
    * as the underlying memory block that was passed to the constructor
    * is in scope somewhere.
    */
    const_data_it begin() const
    { return _begin_it; }

    /** Get ownership to message data.
    * This function returns the ownership object that ensures that the data
    * iterator pointed to by getDataIterator() is valid.
    *
    * @returns An ownership object ensuring that a pointer returned by
    * begin() is valid.
    */
    std::shared_ptr<const byte_traits::byte_sequence> getOwnership() const
    { return _memblock; }
};

/** Base class for SegmentationLayer.
 * Template parameter independent typedefs and functions for the
 * SegmentationLayer<> template class.
 */
struct SegmentationLayerBase
{
    /** Layer identifier */
    static constexpr byte_traits::byte_t LAYER_ID = 0x80;

    /** Header length */
    static constexpr std::size_t header_length = 4;

    /** Type representing the header of a packet. */
    struct HeaderType {
        byte_traits::uint2b_t packetsize /**< Size of the packet */;
    };


    /** Header decoding function.
    *
    * Creates a HeaderType structure from a series of bytes. The header is
    * checked for validity, if it is invalid an InvalidHeaderError is thrown.
    *
    * @tparam InputIterator An Iterator type whos dereferenced type  is
    * convertible to byte_traits::byte_t. Must meet the
    * InputIterator requirement.
    *
    * @param headerbuf An Iterator to series of bytes containing the header of
    * a serialized SegmentationLayer message. Must be at header_length bytes
    * long.
    */
    template <typename InputIterator>
    static HeaderType decodeHeader(InputIterator headerbuf);
};



/** Layer ensuring correct segmentation of messages
*
* This class should be used as the lowest layer of a message - the one that
* shall be sent over the network. The next level lower than this one is the TCP
* layer.
* This class tags messages with a binary header in the following layout:
* Bits
* 0:      Layer Identifier, Value 0x80
* 1-2:    Packet size in Network Byte Order
* 3:      Zero, Value 0x0
*
*/
template <typename InnerLayer>
struct SegmentationLayer : public SegmentationLayerBase
{
    /** Message of an inner layer */
    InnerLayer _inner_layer;

    /** Copy constructor.
    * Constructs a new object from another objectby  copying the content.
    *
    * @param other Object to copy data from
    */
    explicit SegmentationLayer(const SegmentationLayer&) = default;

    /** Copy assignment operator.
    * Constructs a new object from another objectby  copying the content.
    *
    * @param other Object to copy data from
    */
    SegmentationLayer& operator= (const SegmentationLayer&) = default;

    /** Move constructor.
    * Transfers ownership of the data contained in other.
    *
    * @note After invocation of this constructor, other will be invalidated and
    * should not be used.
    */
    SegmentationLayer(SegmentationLayer&& other)
        : _inner_layer{std::move(other._inner_layer)}
    {
        // workaround for gcc bug
        // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=51629
        // This should be "= default"
    }

    /** Move assignment operator.
    * Transfers ownership of the data contained in other.
    *
    * @note After invocation of this operator, other will be invalidated and
    * should not be used.
    */
    SegmentationLayer& operator= (SegmentationLayer&&) = default;

    /** Construct from InnerLayer object.
    * Transfers ownership of the data contained in the upper layer object and
    * wraps its data into *this.
    *
    * @note After invocation of this constructor, upper_layer will be
    * invalidated and should not be used.
    */
    SegmentationLayer(InnerLayer&& upper_layer)
        : _inner_layer{std::move(upper_layer)}
    { }

    // overriding base class version
    std::size_t size() const
    { return _inner_layer.size() + header_length; }

    // overriding base class version
    template <typename ByteOutputIterator>
    ByteOutputIterator fillSerialized(ByteOutputIterator it) const;
};



/** Layer wrapping a string.
* This class is a simple wrapper around a wstring message.
* No header is prepended to the message.
*/
struct StringwrapLayer : public BasicMessageLayer<StringwrapLayer>
{
    /** The actual text message */
    byte_traits::msg_string _message_string;

    /** Default constructor. */
    StringwrapLayer() = default;

    /** Copy constructor */
    explicit StringwrapLayer(const StringwrapLayer& msg) = default;

    /** Default assignment operator */
    StringwrapLayer& operator= (const StringwrapLayer& msg) = default;

    /** Move constructor.
     * Create a StringwrapLayer message from a temporary StringwrapLayer object
     *
     * @param other The other StringwrapLayer object you want to steal from
     *
     * @note After invocation of this constructor, other will be invalidated and
     * should not be used.
    */
    StringwrapLayer(StringwrapLayer&& other) = default;

    /** Move assignment operator.
    * Transfers ownership of the message contained in other.
    *
    * @note After invocation of this constructor, other will be invalidated and
    * should not be used.
    */
    StringwrapLayer& operator= (StringwrapLayer&& msg) = default;

    /** Constructor.
    * Create a StringwrapLayer message from an byte_traits::msg_string.
    *
    * @param msg msg The string the message shall contain.
    */
    StringwrapLayer(const byte_traits::msg_string& msg) throw ()
        : _message_string{msg}
    {}

    /** Constructor.
    * Create a StringwrapLayer message from an byte_traits::msg_string.
    *
    * @param msg msg The string the message shall contain.
    */
    StringwrapLayer(byte_traits::msg_string&& msg) throw ()
        : _message_string{std::move(msg)}
    {}


    /** Constructor.
    * Create a StringwrapLayer from a message of unknown message layer, for
    * example a message that comes from the network.
    * This constructor parses a bytewise buffer into a widestring and loads this
    * string into the internal buffer.
    * If the number of bytes in the byte sequence after the header is not a
    * multiple of the character size, an exception is thrown.
    *
    * @param msg Message of unknown message layer type.
    * @throws MsgLayerError if the bytesize is not a multiple of the character
    * size.
    */
    StringwrapLayer(const SerializedData& msg);

    // overriding base class version
    std::size_t size() const
    {
        return _message_string.length() *
            sizeof(byte_traits::msg_string::value_type);
    }

    // overriding base class version
    template <typename ByteOutputIterator>
    ByteOutputIterator fillSerialized(ByteOutputIterator it) const;
};


template <typename InputIterator>
SegmentationLayerBase::HeaderType
SegmentationLayerBase::decodeHeader(InputIterator headerbuf)
{
    HeaderType headerdata;

    // check first byte to be the correct layer identifier
    if ( headerbuf[0] !=
        static_cast<byte_traits::byte_t>(SegmentationLayerBase::LAYER_ID) )
        throw InvalidHeaderError();

    // get the size of the packet
    readbytes<byte_traits::uint2b_t>(&headerdata.packetsize, &headerbuf[1]);

    headerdata.packetsize = to_hostbo(headerdata.packetsize);

    if (headerbuf[3] != 0)
        throw InvalidHeaderError{};

    return headerdata;
}

extern template
SegmentationLayerBase::HeaderType SegmentationLayerBase::decodeHeader(
    byte_traits::byte_sequence::iterator headerbuf);


// overriding base class version
template <typename InnerLayer>
template <typename ByteOutputIterator>
ByteOutputIterator
SegmentationLayer<InnerLayer>::fillSerialized(ByteOutputIterator it) const
{
    // first byte is layer identifier
    *it++ = static_cast<byte_traits::byte_t>(LAYER_ID);

    // second and third bytes are the size of the whole packet
    it = writebytes(it, to_netbo(
        static_cast<byte_traits::uint2b_t>(_inner_layer.size()+header_length)));

    // fourth byte is a zero
    *it++ = 0;

    // the rest is the message
    return _inner_layer.fillSerialized(it);
}

template <typename ByteOutputIterator>
ByteOutputIterator StringwrapLayer::fillSerialized(ByteOutputIterator it) const
{
    // write all bytes of one character into the buffer, advance the output
    // iterator
    for (auto in_it  = _message_string.begin(); in_it < _message_string.end(); in_it++)
        it = writebytes(it, to_netbo(*in_it));

    return it;
}

extern template
byte_traits::byte_sequence::iterator
StringwrapLayer::fillSerialized(byte_traits::byte_sequence::iterator it) const;


extern template class BasicMessageLayer<SerializedData>;
extern template class SegmentationLayer<SerializedData>;
extern template class BasicMessageLayer<StringwrapLayer>;
extern template class SegmentationLayer<StringwrapLayer>;




/**@}*/ // addtogroup common

} // namespace nuke_ms

#endif // ifndef MSGLAYERS_HPP_INCLUDED

