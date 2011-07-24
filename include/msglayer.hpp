// msglayer.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2009, 2010  Alexander Korsunsky
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
* encoding and decoding messages into messages of a lower or upper layer.
* To clarify the use of this class hierarchy, one hypothetical procedure of
* sending a message is explained.
*
* The message travels from the user "down the pipeline" to the network, where it
* can be sent.
* At the beginning the user enters some data (e.g. a text message). This data is
* encapsuled in a wrapper tagging the message with metadata (like username,
* font...). Let's call this layer "layer A".
* Then message of layer A is encrypted becomes a message of layer b.
* Finally the message is serialized. To ensure wholeness of the message, it is
* encapsuled in a segmentation layer, layer C.
*
* At this moment the message looks like this:
* layer C object holds a copy of the layer B object, which holds a copy of layer
* A object, which holds a copy of the text the user entered.
* Now, the message can be sent over the network.
*
*
* This is what happens on the receiving site:
* The header of the segmentation layer message (layer C) arrives at the
* receiving site. The receiving site allocates memory and receives the
* rest of the message.
* Then, the message is passed "up the pipeline" to a responsible function
* which determines the correct layer of the message (layer B in our case) and
* passes the message on to the next layer handling function. His layer B
* function unpacks the encapsuled layer A message which is then finally
* displayed back to the user.
*
*
* Implementation
*
* The message pipeline is implemented by a class for each layer. An object of
* this class is therefor considered as a message of this layer.
* Although the above explanation would suggest that each layer holds a copy of
* it's upper layer, it is up to the implementation of a layer to decide wether
* the data is held as a reference or a copy.
* Normally a layer only adds a small header (compared to the size of the
* original message) to the message and passes it down to the next layer. It
* would be waste of memory and computation time to copy the data of each upper
* layer into the current layer.
* To avoid this, layers should hold their data as a reference counted shared
* pointer. Only when necessary - usually when the data is actually sent over the
* network - the data is serialized and written into a single buffer. This buffer
* is sent over the network, and the original data chunks (headers + user data)
* are discarded (and deleted!).
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

/** Type to denote invalid header encoding */
struct InvalidHeaderError : public MsgLayerError
{
    InvalidHeaderError()
        : MsgLayerError("Invalid packet header")
    {}
};

/** Type to denote undersized packet */
struct UndersizedPacketError : public MsgLayerError
{
    UndersizedPacketError()
        : MsgLayerError("Packet too small")
    {}
};


/** Object holding an ownership to memory.
* This class is a capsule around a smart pointer.
* As long as an object of this class exists, its underlying memory block
* (specified in the constructor) also exists.
* Only when all of the objects holding the ownership to one memory block
* go out of scope, the memory block will be deleted.
* The reference counting functionality will be provided by the smart pointer
* type specified as template parameter. Make sure the template parameter is
* a smart pointer, otherwise this class will not work.
*
* Thread safety: equal to the construction and destruction thread safety of the
* smart pointer type.
*
* @tparam PointerType Type of a smart pointer
*/
template <typename PointerType>
class MemoryOwnership
{
    PointerType memory_pointer; /**< A smart pointer to the memory block */

public:
    /** Default constructor.
    * Construct object with no ownership to any memory block.
    * The memory pointer will be default constructed.
    */
    MemoryOwnership() {}

    /** Constructor.
    * Construct object with ownership to the memory block pointed to by
    * _memory_pointer.
    */
    MemoryOwnership(const PointerType& _memory_pointer)
        : memory_pointer(_memory_pointer)
    {}
};


// forward declaration for SerializedData
class SerializedData;

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
class BasicMessageLayer
{
public:
    typedef byte_traits::byte_sequence::iterator data_it;
    typedef byte_traits::byte_sequence::const_iterator const_data_it;

    /** Retrieve the serialized size.
     * Returns the length of the returned sequence of a successive call to
     * serialize().
     *
     * @return The number of bytes the serialized byte sequence would have.
    */
    std::size_t size() const
    { return DerivedType::size(); }


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

/** Alias for a Memory ownership of a std::shared_ptr pointer */
typedef MemoryOwnership<std::shared_ptr<byte_traits::byte_sequence>> DataOwnership;


/** Message of an unknown message layer.
* This class should be used to hold a serialized message of an unknown type.
* It holds only the reference to the memory block and an iterator to the data.
* It provides a possibility to access the contained data to determine the real
* message layer class.
*/
class SerializedData : public BasicMessageLayer<SerializedData>
{
    DataOwnership _memblock; /**< Ownership to the memory block */
    const_data_it _begin_it; /**< Iterater to the beginning data */
    std::size_t _datasize; /**< Size of the data */

public:

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
        DataOwnership memblock,
        const_data_it begin_it,
        std::size_t datasize
    )
        : _memblock(memblock), _begin_it(begin_it), _datasize(datasize)
    {}

    SerializedData(const SerializedData&) = default;

    SerializedData(SerializedData&& other)
        : _memblock(std::move(other._memblock)),
        _begin_it(other._begin_it), _datasize(other._datasize)
    {
        // invalidate iterator of rvalue object by assigning a default
        // constructed one
        other._begin_it = const_data_it{};
        other._datasize = 0;
    }

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
    * SerializedData() or this object is alive.
    */
    const_data_it getDataIterator() const
    { return _begin_it; }

    /** Get ownership to message data.
    * This function returns the ownership object that ensures that the data
    * iterator pointed to by getDataIterator() is valid.
    *
    * @returns An ownership object ensuring that a pointer returned by
    * getDataIterator() is valid.
    */
    DataOwnership getOwnership() const
    { return _memblock; }
};


struct SegmentationLayerBase
{
    static constexpr byte_traits::byte_t LAYER_ID = 0x80;
    static constexpr std::size_t header_length = 4;

    /** Type representing the header of a packet. */
    struct HeaderType {
        byte_traits::uint2b_t packetsize /**< Size of the packet */;
    };


    /** Header decoding function.
    *
    * Creates a HeaderType structure from a series of bytes. The header is
    * checked for validity, if invalid an InvalidHeaderError is thrown.
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
class SegmentationLayer
    : public SegmentationLayerBase,
    public BasicMessageLayer<SegmentationLayer<InnerLayer>>
{
    InnerLayer _inner_layer;

public:
    /** Constructor.
    * Construct a SegmentationLayer message from an upper layer, say a message
    * coming from the application.
    *
    * @note To pass an upper layer, you have to static_cast the pointer to the
    * message to the base class Pointer, BasicMessageLayer::ptr_t. Otherwise
    * the overload with SegmentationLayer(BasicMessageLayer::dataptr_t)
    * will not resolve. This is because overloads of functions with different
    * instantiations of std::shared_ptr<> do not resolve with base class
    * pointers.
    *
    * @param _upper_layer The message of the upper layer you want to send.
    */
    SegmentationLayer(const InnerLayer& upper_layer)
        : _inner_layer(upper_layer)
    { }

    SegmentationLayer(InnerLayer&& upper_layer)
        : _inner_layer(std::move(upper_layer))
    { }

    // overriding base class version
    std::size_t size() const
    { return _inner_layer.size() + header_length; }

    // overriding base class version
    template <typename ByteOutputIterator>
    ByteOutputIterator fillSerialized(ByteOutputIterator it) const;

    InnerLayer&& getUpperLayer()
    { return std::move(_inner_layer); }

    const InnerLayer& getUpperLayer() const
    { return _inner_layer; }
};


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



/** Layer wrapping a string.
* This class is a simple wrapper around a wstring message.
* No header is prepended to the message.
*/
class StringwrapLayer : public BasicMessageLayer<StringwrapLayer>
{
    typedef byte_traits::msg_string StringType;

    /** The actual text message */
    StringType _message_string;

public:
    /** Default Constructor. */
    StringwrapLayer() = default;

    /** Constructor.
    * Create a StringwrapLayer message from an byte_traits::msg_string.
    *
    * @param msg msg The string the message shall contain.
    */
    StringwrapLayer(const StringType& msg) throw ()
        : _message_string(msg)
    {}

    /** Move constructor.
     * Create a StringwrapLayer message from a temporary StringwrapLayer object
     * @param other The other StringwrapLayer object you want to steal from
    */
    StringwrapLayer(StringwrapLayer&& other)
        : _message_string(std::move(other._message_string))
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
        return _message_string.length() *  sizeof(StringType::value_type);
    }

    // overriding base class version
    template <typename ByteOutputIterator>
    ByteOutputIterator fillSerialized(ByteOutputIterator it) const;

    /** Return the string contained in the layer message.
    *
    * This function returns a constant reference to the internal string message.
    * When using this string object bear in mind that this is only a reference,
    * not a copy. This reference is valid as long as *this is alive.
    * If you need to pass this string on, copy construvt a new instance from
    * this reference.
    *
    * @returns A constant reference to the string contained in this message
    */
    const StringType& getString() const
    { return _message_string; }

    /** Return the string contained in the layer message.
    *
    * This function returns a constant reference to the internal string message.
    * When using this string object bear in mind that this is only a reference,
    * not a copy. This reference is valid as long as *this is alive.
    * If you need to pass this string on, copy construct a new instance from
    * this reference.
    *
    * @returns A constant reference to the string contained in this message
    */
    operator const StringType& () const
    { return _message_string; }
};


template <typename ByteOutputIterator>
ByteOutputIterator StringwrapLayer::fillSerialized(ByteOutputIterator it) const
{
    // write all bytes of one character into the buffer, advance the output
    // iterator
    for (auto in_it  = _message_string.begin(); in_it < _message_string.end(); in_it++)
        it = writebytes(it, to_netbo(*in_it));

    return it;
}


/**@}*/ // addtogroup common

} // namespace nuke_ms

#endif // ifndef MSGLAYERS_HPP_INCLUDED
