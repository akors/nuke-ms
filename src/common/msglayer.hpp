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


struct InvalidHeaderError : public MsgLayerError
{
    InvalidHeaderError() throw()
        : MsgLayerError("Invalid packet header")
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
    * @param buffer A series of bytes
    */
    virtual void fillSerialized(data_iterator buffer) const throw() = 0;
};


typedef MemoryOwnership<BasicMessageLayer::dataptr_type> DataOwnership;


class UnknownMessageLayer : public BasicMessageLayer
{
    DataOwnership memblock;
    BasicMessageLayer::const_data_iterator data_it;
    std::size_t data_size;

public:
    UnknownMessageLayer(
        DataOwnership _memblock,
        BasicMessageLayer::const_data_iterator _data_it,
        std::size_t _data_size,
        bool new_memory_block = false
    );

    // overriding base class version
    virtual std::size_t getSerializedSize() const throw();

    // overriding base class version
    virtual void fillSerialized(data_iterator buffer) const throw();

    BasicMessageLayer::const_data_iterator getDataIterator() const throw()
    { return data_it; }
};



} // namespace nuke_ms

#endif // ifndef MSGLAYERS_HPP_INCLUDED
