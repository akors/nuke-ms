// protocol.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2009, 2010  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License.
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
#include <boost/ref.hpp>
#include<boost/tokenizer.hpp>


#include "protocol/statemachine.hpp"
#include "protocol/protocol.hpp"

#include "bytes.hpp"
#include "msglayer.hpp"


/** @defgroup proto_machine Comunication Protocol State Machine
* @ingroup proto */


using namespace nuke_ms;
using namespace protocol;


static bool parseDestinationString(
    byte_traits::native_string& host,
    byte_traits::native_string& service,
    const byte_traits::native_string& where
);


NukeMSProtocol::NukeMSProtocol()
    : machine_scheduler(true), last_msg_id(0)
{

    // create an event processor for our state machine

    // Passing _io_service by pointer, because passing references to
    // create_processor does not work.
#ifdef I_HATE_THIS_DAMN_BUGGY_STATECHART_LIBRARY
    event_processor =
        machine_scheduler.create_processor<ProtocolMachine, Signals&>(
        boost::ref(signals));
#else
    event_processor =
        machine_scheduler.create_processor<ProtocolMachine, Signals*>(
        &signals);
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

NukeMSProtocol::~NukeMSProtocol()
{
    // stop the network machine
    machine_scheduler.terminate();

    // catch the running thread
    catchThread(machine_thread, threadwait_ms);
}


void NukeMSProtocol::connectTo(protocol::ServerLocation::const_ptr_t where)
{
    // Get Host/Service pair from the destination string
    byte_traits::native_string host, service;
    if (parseDestinationString(host, service, where->where))
    {  // on success, pass on event
        // Create new Connection request event
        // and dispatch it to the statemachine
        boost::intrusive_ptr<EvtConnectRequest>
        connect_request(new EvtConnectRequest(host, service));

        machine_scheduler.queue_event(event_processor, connect_request);
    }
    else // on failure, report back to application
    {
        using namespace protocol;
        ConnectionStatusReport::ptr_t rprt(new ConnectionStatusReport);
        rprt->newstate = ConnectionStatusReport::CNST_DISCONNECTED;
        rprt->statechange_reason = ConnectionStatusReport::STCHR_CONNECT_FAILED;
        rprt->msg = "Invalid remote site identifier";

        signals.connectStatReport(rprt);
    }

}



NearUserMessage::msg_id_t NukeMSProtocol::sendUserMessage(
    const byte_traits::msg_string& msg,
    const UniqueUserID& recipient
)
{
    NearUserMessage::ptr_t usermsg(new NearUserMessage(msg, recipient));
    usermsg->msg_id = getNextMessageId();

    // Create new Connection request event and dispatch it to the statemachine
    boost::intrusive_ptr<EvtSendMsg> send_evt(new EvtSendMsg(usermsg));
    machine_scheduler.queue_event(event_processor, send_evt);

    return usermsg->msg_id;
}



void NukeMSProtocol::disconnect()
{
    // Create new Disconnect request event and dispatch it to the statemachine
    boost::intrusive_ptr<EvtDisconnectRequest>
    disconnect_evt(new EvtDisconnectRequest);

    machine_scheduler.queue_event(event_processor, disconnect_evt);
}


/** Get host and service pair from a single destination string.
* Parses the string containing the destination into a host/service pair.
* The part before the column is the host, the part after the column is the
* service.
* If there is not exactly one column, the function returns false, indicating
* failure.
*
* @param host A reference to a string where the host will be stored.
* Any content will be overwritten.
* @param service A reference to a string where the service will be stored.
* Any content will be overwritten.
* @param where A string containing a destination.
*
* @return true on success, false on failure.
*/
static bool parseDestinationString(
    byte_traits::native_string& host,
    byte_traits::native_string& service,
    const byte_traits::native_string& where
)
{
    // get ourself a tokenizer
    typedef boost::tokenizer<
        boost::char_separator<wchar_t>,
        byte_traits::native_string::const_iterator,
        byte_traits::native_string
    >
        tokenizer;

    // get the part before the colon and the part after the colon
    boost::char_separator<wchar_t> colons(L":");
    tokenizer tokens(where, colons);

    tokenizer::iterator tok_iter = tokens.begin();

    // bail out, if there were no tokens
    if ( tok_iter == tokens.end() )
        return false;

    // create host from first token
    host.assign(tok_iter->begin(), tok_iter->end());

    if ( ++tok_iter == tokens.end() )
        return false;

    // create service from second token
    service.assign(tok_iter->begin(), tok_iter->end());

    // bail out if there is another colon
    if ( ++tok_iter != tokens.end() )
        return false;

    return true;
}



void nuke_ms::protocol::catchThread(boost::thread& thread, unsigned threadwait_ms)
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
