// clientnode.hpp

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

/** @file clientnode/clientnode.hpp
* @brief Network communication protocol.
* @ingroup clientnode
*
* This file contains all of the networking logic.
* The main class is nuke_ms::clientnode::ClientNode. Use this file within an
* Application Control entitiy.
* This file depends only on the file "notifications.hpp" that contains the
* types for the interface to the Application Control entitiy.
*
* @author Alexander Korsunsky
*/



#ifndef CLIENTNODE_HPP_
#define CLIENTNODE_HPP_

#include <boost/asio.hpp>

#include <boost/thread/thread.hpp>
#include <boost/signals2/signal.hpp>

#include "bytes.hpp"
#include "neartypes.hpp"
#include "clientnode/sigtypes.hpp"
#include "clientnode/statemachine.hpp"



namespace nuke_ms
{

/** @defgroup clientnode Communication Protocol
 *
 * This Module is used as the client side of a server-client connection.
 * It handles lookup, connect/disconnect operations as well as sending and
 * receiving data from/to a server.
 *
 * The main class of this module is the @ref clientnode::ClientNode class, please refer to its
 * documentation for information on how to use this module.
 *
 * The ClientNode is supposed to be reentrant, so in theory you should be able
 * to create multiple instances of it without the possibility of interference
 * between them.
 *
 * @{
*/

namespace clientnode
{


/** Client Communication Protocol.
 *
 * Use this class to create a black box that handles all the network stuff for
 * you.
 *
 * To use it, simply create an instance of this class using the constructor
 * ClientNode().
 * Upon creation, the ClientModule will be initialized and ready to connect to
 * a server.
 * Then you will have to set up callbacks that inform you of the
 * result of connection attempts, send attempts and incoming messages. To
 * register callbacks use the functions connectRcvMessage(),
 * connectConnectionStatusReport() and connectSendReport().
 * You can then post connection/disconnection requests
 * (connectTo(), disconnect()) and send messages (sendUserMessage()).
 *
*/
class ClientNode
{
public:
    /** Constructor.
	 *
    * Creates a thread and initializes the Network machine.
    */
    ClientNode(LoggingStreams logstreams_ = LoggingStreams());

    /** Destructor.
    * Stops the Network machine and destroys the thread.
    */
    ~ClientNode();

	/** Connect the signal for incoming messages.
	 *
	 * @param slot The slot you want to connect the signal to
	 * @return Object to the connection of the signal/slot
	 *
	 * @note The signal passes a non-const shared_ptr to a NearUserMessage
	 * object. If you want to connect to the slot more than once, it is your
	 * responsibility to ensure the validity of the object between slot
	 * invocations.
	*/
    boost::signals2::connection
    connectRcvMessage(const SignalRcvMessage::slot_type& slot)
    { return signals.rcvMessage.connect(slot); }

	/** Connect the signal for connection status reports
	 *
	 * @param slot The slot you want to connect the signal to
	 * @return Object to the connection of the signal/slot
	*/
    boost::signals2::connection connectConnectionStatusReport(
        const SignalConnectionStatusReport::slot_type& slot)
    { return signals.connectStatReport.connect(slot); }

	/** Connect the signal for send reports
	 *
	 * @param slot The slot you want to connect the signal to
	 * @return Object to the connection of the signal/slot
	*/
    boost::signals2::connection
    connectSendReport(const SignalSendReport::slot_type& slot)
    { return signals.sendReport.connect(slot); }


    /** Connect to a remote site.
     * @param where The string representation of the address of the remote site
     */
    void connectTo(const ServerLocation& where);


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

    /** The Streams used for message output */
    LoggingStreams logstreams;

    /** Our state machine */
    ClientnodeMachine statemachine;

    /** A mutex to gain access to the state machine */
    boost::mutex machine_mutex;

    /** The function object that will be called, if an event occurs.*/
    ClientNodeSignals signals;

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
/**@}*/ // defgroup clientnode

} // namespace nuke_ms

#endif /*CLIENTNODE_HPP_*/
