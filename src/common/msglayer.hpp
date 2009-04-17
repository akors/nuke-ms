// msglayer.hpp

/*
 *   nuke-ms - Nuclear Messaging System
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


/** @file msglayer.hpp
* @brief Message Layers
*
* When data is intended to be sent over the network, the data normally is
* encapsuled in many layers, several "headers" are attached to it.
*
* The idea behind this class hierarchy is to ease and unify the handling of
* encoding and decoding messages into messages of a lower or upper layer.
*
* To clarify the use of this hierarchy, one hypothetical procedure of sending a
* message is explained:
* The user enters some data (e.g. a text message). This data is
* encapsuled in a wrapper tagging the message with metadata (like username,
* font...). Let's call this layer "layer A".
* Then message of layer A is encrypted becomes a message of layer b.
* Finally the message is serialized. To ensure wholeness of the message, it is
* encapsuled in a segmentation layer, layer C.
*
* The layer C object holds a copy of the layer B object, which holds a copy of
* layer A object, which holds a copy of the text the user entered.
*
* To be able to send this message, the size of the message has to be determined
* and the data has to be serialized.
* To determine the size, the request is made "up the pipeline":
* layer C asks layer B, which in turn asks A which knows
* the size of the original message. The value travels back "down the pipeline",
* where at each step the header size of each layer is added.
* To serialize the message, layer C serializes its header and asks B to
* serialize. B in turn serializes its header and asks A to serialize. A
* serializes its header and appends the text data.
*
* This message pipeline is implemented by a class for each layer. An object of
* this class is therefor considered as a message of this layer.
* Although the above explanation would suggest that each layer holds a copy of
* it's upper layer, it is up to the implementation of a layer to decide wether
* the data is held as a reference or a copy.
* Normally a layer only adds a small header (compared to the size of the
* original message). Therefor it would be waste of memory and computation time
* to copy the data of each upper layer into the current layer.
* To avoid this, layers should hold their data as a reference counted shared
* pointer. Only when necessary - usually when the data is actually sent over the
* network - the data is serialized and written into a single buffer. This buffer
* is sent over the network, and the original data chunks (headers + user data)
* are discarded (and deleted!).
*
* Let's examine the handling of the message on the receiving side:
* The header of the segmentation layer message (layer C) arrives at the
* receiving site. Then the receiving site allocates memory and receives the
* rest of the message.
* Then, the message is passed "up the pipeline". However, the code that handles
* new messages doesn't know what kind of message (which layer) it has received.
* Therefor, layer C is holding a reference to a message of an "unknown message
* layer". The layer C message is passed to the next handling code. This
* code inspects the "unknown message layer" message and tries to make sense of
* it. If it does, the appropriate message layer (let's say it's layer B) is
* constructed from the unknown message layer. The upper layer contained in this
* layer B message is still unknown, and will be a "unknown message layer"
* message. The message is passed up to the next handling code, the data is
* investigated and the appropriate layer (let's say layer A) is constructed.
* The string message contained in layer A is then displayed to the user.
*
* The main memory allocation for an incoming message should be performed at the
* very beginning, right before receiving the message body of the segmentation
* layer. The whole block should be kept in memory and the message layers pass
* ownership to this block and iterators upwards. This is a tradeoff:
* At the expense of unnessecary bytes being held in memory, additional memory
* allocations and copy operations can be prevented. This should be a good
* tradeoff because "normally" the headers on the messages are relatively small
* and memory waste is negelctable.
* If for a layer, the cost becomes to high (if its header is rather big), it
* can decide to discard the old buffer and allocate a new smaller buffer.
*
*/


namespace nuke_ms
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
* Thread safety: equal to the construction and destruction
* thread safety of the smart pointer type.
*
* @tparam PointerType Type of a smart pointer
*/
template <typename PointerType>
class MemoryOwnership
{
    PointerType memory_pointer;

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


/** Message Layer base class.
* This virtual base class presents a basic interface to it's deriving classes.
* Derived classes are said to represent "layers" of the communication pipeline,
* and objects of these deriving classes represent "messages" of their layer
* in the communication pipeline.
*
*
*/
class BasicMessageLayer
{
public:
    typedef boost::shared_ptr<BasicMessageLayer> ptr_type;
    typedef boost::shared_ptr<byte_traits::byte_sequence> dataptr_type;

    typedef byte_traits::byte_sequence::iterator data_iterator;
    typedef byte_traits::byte_sequence::const_iterator const_data_iterator;

    /** Virtual destructor */
    virtual ~BasicMessageLayer()
    {}

    /** Retrieve the serialized size.
     * Returns the length of the returned sequence of a successive call to
     * serialize(). <br>
     *
     * @return The number of bytes the serialized byte sequence would have.
    */
    virtual std::size_t getSerializedSize() const throw() = 0;

    /** Fill a buffer with the serialized version of this object.
    * This function serializes itself (and its upper layers) and writes
    * the bytes into a buffer that is pointed to by buffer.
    * The buffer has to have at least the size that is returned by the
    * getSerializedSize() functions. Beware! No checks will be performed
    * to assure proper buffer size. Use the function getSerializedSize() to
    * obtain the minimal required size.
    *
    * @param buffer An iterator pointing to a range in the buffer that will be
    * filled
    * @returns An iterator pointing past the filled range in the buffer. This
    * is the same iterator as buffer but it is incremented getSerializedSize()
    * times.
    */
    virtual data_iterator fillSerialized(data_iterator buffer) const throw()= 0;
};


typedef MemoryOwnership<BasicMessageLayer::dataptr_type> DataOwnership;

/** Message of an unknown message layer.
* This class should be used when a new message arrives and its real message
* layer class is not determined yet. It holds all only the reference to the
* memory block and a iterator to the data.
* It provides a possibility to access the contained data to determine the real
* message layer class.
*/
class UnknownMessageLayer : public BasicMessageLayer
{
    typedef boost::shared_ptr<UnknownMessageLayer> ptr_type;

    DataOwnership memblock; /**< Ownership to the memory block */
    const_data_iterator data_it; /**< Iterater to the data */
    std::size_t data_size; /**< Size of the data */

public:
    /** Constructor.
    * Constructs a new object and passes memory ownership, data iterator and
    * data size. Note that the memory block can contain other data than is
    * accessible with the data iterator. If desired, a new block of memory can
    * be allocated by setting the new_memory_block parameter to true.
    *
    * @param _memblock Ownership of a memory block that ensures that _data_it is
    * valid
    * @param _data_it Iterator to the data of the message in the memory block
    * block
    * @param _data_size size of the message in bytes
    */
    UnknownMessageLayer(
        DataOwnership _memblock,
        const_data_iterator _data_it,
        std::size_t _data_size,
        bool new_memory_block = false
    );

    // overriding base class version
    virtual std::size_t getSerializedSize() const throw();

    // overriding base class version
    virtual data_iterator fillSerialized(data_iterator buffer) const throw();

    /** Get iterator to message data.
    * This function can be used to access the buffer directly, so that
    * other layers can construct themselves from data coming from the network.
    *
    * @returns An iterator to the message data. This iterator is valid as long
    * as this object is alive.
    */
    const_data_iterator getDataIterator() const throw()
    { return data_it; }

    /** Get ownership to message data.
    * This function returns the ownership object that ensures that the data
    * iterator pointed to by getDataIterator() is valid.
    *
    * @returns An ownership object ensuring that a pointer returned by
    * getDataIterator() is valid.
    *
    */
    DataOwnership getOwnership() const throw()
    { return  memblock; }
};

/** Layer wrapping a string.
* This class is a simple wrapper around a wstring message. The header of this
* layer encodes only the bytesize of a character text.
*
* The header looks like the following:
*
*  0 1 2 3 4 5
* +-+-+-+-+-+-+
* |I|S|Text ...
* +-+-+-+-+-+-+
*
* Byte 0: Layer identifier of this layer ( 0x81 )
* Byte 1: Bytesize of a character of the text
* Byte 2 to end: Text of the message with character
*
* The text field must have a length such that the number of bytes module the
* character bytesize equals zero.
*/
class StringwrapLayer : public BasicMessageLayer
{
    /** The actual text message */
    std::wstring message_string;

    /** size of the character in bytes */
    std::size_t charsize;

    /** header length of this layer */
    enum { header_length = 2 };
public:
    /** Typedef for this shared pointer*/
    typedef boost::shared_ptr<StringwrapLayer> ptr_type;

    /** Layer identifier */
    enum { LAYER_ID = 0x81 };


    /** Constructor.
    * Create a StringwrapLayer message from an std::wstring.
    *
    * @param msg msg The string the message shall contain.
    */
    StringwrapLayer(const std::wstring& msg) throw ();

    /** Constructor.
    * Create a StringwrapLayer from a message of unknown message layer, for
    * example a message that comes from the network.
    * This constructor parses a bytewise buffer into a widestring and loads this
    * string into the internal buffer.
    * If the number of bytes in the byte sequence after the header is not a
    * multiple of the character size declared in the header of the message,
    * an exception is thrown.
    *
    * @param msg Message of unknown message layer type.
    * @throws MsgLayerError when the data in msg does not conform to the
    * layer encoding definition.
    *
    */
    StringwrapLayer(const UnknownMessageLayer& msg) throw(MsgLayerError);

    // overriding base class version
    virtual std::size_t getSerializedSize() const throw();

    // overriding base class version
    virtual data_iterator fillSerialized(data_iterator buffer) const throw();

    /** Return the string contained in the layer message.
    *
    * This function returns a constant reference to the internal string message.
    * When using this string object bear in mind that this is only a reference,
    * not a copy. This reference is valid as long as *this is alive.
    * If you need to pass this string on, create a new instance from this
    * reference.
    *
    * @returns A constant reference to the string contained in this message
    */
    const std::wstring &getString() const throw()
    { return message_string; }

    /** Get bytesize of a character.
    * This function returns the size of a single character of the message.
    * Note that this function is only useful when inspecting an incoming
    * message. For outgoing messages, the charactersize will allways be equal
    * to sizeof(std::wstring::value_type)
    */
    std::size_t getCharsize() const throw()
    { return charsize; }

};



} // namespace nuke_ms

#endif // ifndef MSGLAYERS_HPP_INCLUDED
