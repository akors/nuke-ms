// clientnode.cpp

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
#include <boost/tokenizer.hpp>

#include "clientnode/clientnode.hpp"

#include "bytes.hpp"


using namespace nuke_ms;
using namespace clientnode;


static bool parseDestinationString(
    byte_traits::native_string& host,
    byte_traits::native_string& service,
    const byte_traits::native_string& where
);


ClientNode::ClientNode(LoggingStreams logstreams_)
    : statemachine(signals, logstreams, machine_mutex),
    last_msg_id(0),  logstreams(logstreams_)
{
    // initiate the event processor
    statemachine.initiate();
}

ClientNode::~ClientNode()
{
    // stop the network machine
    statemachine.terminate();
}


void ClientNode::connectTo(ServerLocation::const_ptr_t where)
{
    // Get Host/Service pair from the destination string
    byte_traits::native_string host, service;
    if (parseDestinationString(host, service, where->where))
    {  // on success, pass on event

        // lock the mutex to the machine, process event
        boost::mutex::scoped_lock(machine_mutex);
        statemachine.process_event(EvtConnectRequest(host, service));
    }
    else // on failure, report back to application
    {
        using namespace clientnode;
        ConnectionStatusReport::ptr_t rprt(new ConnectionStatusReport);
        rprt->newstate = ConnectionStatusReport::CNST_DISCONNECTED;
        rprt->statechange_reason = ConnectionStatusReport::STCHR_CONNECT_FAILED;
        rprt->msg = "Invalid remote site identifier";

        signals.connectStatReport(rprt);
    }

}



NearUserMessage::msg_id_t ClientNode::sendUserMessage(
    const byte_traits::msg_string& msg,
    const UniqueUserID& recipient
)
{
    NearUserMessage::ptr_t usermsg(new NearUserMessage(msg, recipient));
    usermsg->msg_id = getNextMessageId();

    // lock the mutex to the machine
    boost::mutex::scoped_lock(machine_mutex);
    statemachine.process_event(EvtSendMsg(usermsg));

    return usermsg->msg_id;
}



void ClientNode::disconnect()
{
    // lock the mutex to the machine, dispatch disconnect request
    boost::mutex::scoped_lock(machine_mutex);
    statemachine.process_event(EvtDisconnectRequest());
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



void nuke_ms::clientnode::catchThread(boost::thread& thread, unsigned threadwait_ms)
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
