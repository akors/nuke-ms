// protocol.cpp

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


#include <boost/bind.hpp>

#include "protocol/statemachine.hpp"
#include "protocol/protocol.hpp"

#include "bytes.hpp"
#include "msglayer.hpp"


/** @defgroup proto_machine Comunication Protocol State Machine
* @ingroup proto */


using namespace nms;
using namespace protocol;



NMSProtocol::NMSProtocol(const control::notif_callback_t _notification_callback)
            throw()

    : machine_scheduler(true), notification_callback(_notification_callback)
{

    std::cout<<"Protocol constructed.\n";

#if 0
    // create an event processor for our state machine
    event_processor =
        machine_scheduler.create_processor<
            ProtocolMachine,
            control::notif_callback_t,
            boost::asio::io_service&
        >(notification_callback, io_service);
#else
    // create an event processor for our state machine
    event_processor =
        machine_scheduler.create_processor<
            ProtocolMachine,
            control::notif_callback_t
        >(notification_callback);
#endif

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
    // Create new Connection request event and dispatch it to the statemachine
    boost::intrusive_ptr<EvtConnectRequest>
    connect_request(new EvtConnectRequest(id));

    machine_scheduler.queue_event(event_processor, connect_request);
}



void NMSProtocol::send(const std::wstring& msg)
    throw(std::runtime_error, ProtocolError)
{
    // Create new Connection request event and dispatch it to the statemachine
    boost::intrusive_ptr<EvtSendMsg>
    send_evt(
        new EvtSendMsg(msg)
    );

    machine_scheduler.queue_event(event_processor, send_evt);
}


void NMSProtocol::disconnect()
    throw(std::runtime_error, ProtocolError)
{
    // Create new Disconnect request event and dispatch it to the statemachine
    boost::intrusive_ptr<EvtDisconnectRequest>
    disconnect_evt(new EvtDisconnectRequest);

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
