// clientnode.hpp

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

/** @file clientnode.hpp
* @brief Network communication protocol.
*
* This file contains all of the networking logic.
* The main class is nuke_ms::clientnode::ClientNode. Use this file within an
* Application Control entitiy.
* This file depends only on the file "notifications.hpp" that contains the
* types for the interface to the Application Control entitiy.
*
* @author Alexander Korsunsky
*/

/** @defgroup proto Communication Protocol */



#ifndef CLIENTNODE_HPP_
#define CLIENTNODE_HPP_

#include <stdexcept>

#include <boost/asio.hpp>

#include <boost/thread/thread.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/statechart/asynchronous_state_machine.hpp>

#include "bytes.hpp"
#include "neartypes.hpp"
#include "clientnode/sigtypes.hpp"



namespace nuke_ms
{
namespace clientnode
{




/** Client Communication Protocol.
* @ingroup proto
* Use this class to create a black box that handles all the network stuff for
* you.
* Request are handled by the public functions connect_to, send and disconnect.
* Replies will be dispatched to the callback function you supply in the constructor.
*/
class ClientNode
{
public:
    struct Signals
    {
        SignalRcvMessage rcvMessage;
        SignalConnectionStatusReport connectStatReport;
        SignalSendReport sendReport;
    };

    /** Constructor.
    * Creates a thread and initializes the Network machine.
    */
    ClientNode();

    /** Destructor.
    * Stops the Network machine and destroys the thread.
    */
    ~ClientNode();

    boost::signals2::connection
    connectRcvMessage(const SignalRcvMessage::slot_type& slot)
    { return signals.rcvMessage.connect(slot); }

    boost::signals2::connection connectConnectionStatusReport(
        const SignalConnectionStatusReport::slot_type& slot)
    { return signals.connectStatReport.connect(slot); }

    boost::signals2::connection
    connectSendReport(const SignalSendReport::slot_type& slot)
    { return signals.sendReport.connect(slot); }


    /** Connect to a remote site.
     * @param id The string representation of the address of the remote site
     */
    void connectTo(ServerLocation::const_ptr_t where);


    /** Send message to connected remote site.
     *
     * This will send the user message to the recipient specified.
     * If recipient is set to UniqueUserID::user_id_none, the
     * message will be sent to all clients connected to the server.
     *
     * @param msg The message you want to send
     * @param recipient Recipient of the message
     * @return the message identifier of the sent message
     */
    NearUserMessage::msg_id_t sendUserMessage(
        const byte_traits::msg_string& msg,
        const UniqueUserID& recipient = UniqueUserID()
    );


    /** Disconnect from the remote site.
    */
    void disconnect();

private:

    /** Retrieve new unique message identifier.
     * This identifier will be unique on this client
     * @return unique message identifier
    */
    NearUserMessage::msg_id_t getNextMessageId()
    {
        // for now this is a simple increment.
        // Maybe in the future there will be a need for more sophisticated
        // identifier algorithms.
        return ++last_msg_id;
    }

    /** An own thread for the State Machine*/
    boost::thread machine_thread;

    /** The scheduler for the asynchronous state machine */
    boost::statechart::fifo_scheduler<> machine_scheduler;

    /** The event processor handle for the state machine */
    boost::statechart::fifo_scheduler<>::processor_handle event_processor;

    /** The function object that will be called, if an event occurs.*/
    Signals signals;

    /** The I/O Service object used by all network operations */
    boost::asio::io_service io_service;

    /** Unique message identifier of the last message */
    NearUserMessage::msg_id_t last_msg_id;

    /** How long to wait for the thread to join */
    enum { threadwait_ms = 3000 };
};



/** Catch a running thread.
 * This function tries to catch a running thread, by waiting for him
 * threadwait_ms milliseconds to join. If it doesnt join, it is interrupted.
 * If the thread is then still running, the thread is detached.
 *
 * @param thread A reference to the thread you want to catch.
 * @param threadwait_ms The number of milliseconds you want to wait for the
 * thread.
 * @throws Nothing.
 * @post thread does not refer to a thread anymore.
 *
*/
void catchThread(boost::thread& thread, unsigned threadwait_ms);



} // namespace clientnode
} // namespace nuke_ms

#endif /*CLIENTNODE_HPP_*/
