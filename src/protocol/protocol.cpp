// protocol.cpp

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


#include <boost/bind.hpp>

#include "protocol/statemachine.hpp"
#include "protocol/protocol.hpp"

#include "bytes.hpp"
#include "msglayer.hpp"


/** @defgroup proto_machine Comunication Protocol State Machine
* @ingroup proto */



using namespace nms;
using namespace protocol;



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



BasicMessageLayer::dataptr_type StringwrapLayer::serialize() const throw()
{
    BasicMessageLayer::dataptr_type data(
        new byte_traits::byte_sequence(payload)
    );

    return data;
}

BasicMessageLayer::dataptr_type StringwrapLayer::getPayload() const throw()
{
    return this->serialize();
}


StringwrapLayer::StringwrapLayer(const std::wstring& msg) throw ()
    : payload(msg.length()*sizeof(std::wstring::value_type))
{
    // initialize payload to be as big as sizeof(std::wstring::value_type)
    // times the message length

    // one iterator for input, one for output
    byte_traits::byte_sequence::iterator out_iter = payload.begin();
    std::wstring::const_iterator in_iter = msg.begin();

    // write all bytes of one character into the buffer, advance the output
    // iterator
    for (; in_iter < msg.end(); in_iter++)
        out_iter = writebytes(out_iter, *in_iter);
}



std::wstring StringwrapLayer::getString() const throw()
{
    // assure that the payload length is a multiple of the
    // size of the character type (granted by the constructor)
    assert(!(payload.size() % sizeof(std::wstring::value_type)));

    // creat a string, initialize with the right size
    // and create iterator pointing at it
    std::wstring str((payload.size()/sizeof(std::wstring::value_type)), 'a');
    std::wstring::iterator out_iter = str.begin();

    // create a temporary value to store one wide character
    std::wstring::value_type tmpval;

    // pointer to the temporary value where the bytes will be written
    byte_traits::byte_t* tmpval_ptr;

    // iterate through all bytes in the sequence
    for (byte_traits::byte_sequence::const_iterator in_iter = payload.begin();
        in_iter < payload.end();)
    {
        // fill up the temporary character.
        // when the character is full, write it to the string.
        // move the byte pointer forward in each iteration
        for (tmpval_ptr = reinterpret_cast<byte_traits::byte_t*>(&tmpval);
            tmpval_ptr < reinterpret_cast<byte_traits::byte_t*>(&tmpval) + sizeof(tmpval);
            tmpval_ptr++, in_iter++)
        {
            *tmpval_ptr = *in_iter;
        }

        *out_iter++ = tmpval;
    }

    return str;
}





NMSProtocol::NMSProtocol(const control::notif_callback_t _notification_callback)
            throw()

    : machine_scheduler(true), notification_callback(_notification_callback)
{
    // create an event processor for our state machine
    event_processor =
        machine_scheduler.create_processor<ProtocolMachine>(
            _notification_callback
            );

    // initiate the event processor
    machine_scheduler.initiate_processor(event_processor);

    // machine_thread is in state "not-a-thread", so we will use the move
    // semantics provided by boost::thread to create a new thread and assign it
    // the variable
    machine_thread = boost::thread(
        boost::bind(
            &boost::statechart::fifo_scheduler<>::operator(),
            &machine_scheduler,
            0
            )
        );
}

NMSProtocol::~NMSProtocol()
{
    // stop the network machine
    machine_scheduler.terminate();

    // catch the running thread
    catchThread(machine_thread, threadwait_ms);
}


void NMSProtocol::connect_to(const std::wstring& id)
    throw(std::runtime_error, ProtocolError)
{
    boost::intrusive_ptr<EventConnectRequest>
    connect_request(new EventConnectRequest(id));

    machine_scheduler.queue_event(event_processor, connect_request);
}



void NMSProtocol::send(const std::wstring& msg)
    throw(std::runtime_error, ProtocolError)
{
    boost::intrusive_ptr<EventSendMsg>
    send_evt(new EventSendMsg(msg));

    machine_scheduler.queue_event(event_processor, send_evt);
}


void NMSProtocol::disconnect()
    throw(std::runtime_error, ProtocolError)
{
    boost::intrusive_ptr<EventDisconnectRequest>
    disconnect_evt(new EventDisconnectRequest);

    machine_scheduler.queue_event(event_processor, disconnect_evt);
}




void nms::protocol::catchThread(boost::thread& thread, unsigned threadwait_ms)
    throw()
{
    // a thread id that compares equal to "not-a-thread"
    boost::thread::id not_a_thread;

    try {
        // give the thread a few seconds time to join
        thread.timed_join(boost::posix_time::millisec(threadwait_ms));
    }
    catch(...)
    {}

    // if the thread finished, return. otherwise try to kill the thread
    if (thread.get_id() == not_a_thread)
        return;

    thread.interrupt();

    // if it is still running, let it go
    if (thread.get_id() == not_a_thread)
        return;

    thread.detach();
}
