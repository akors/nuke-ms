// protocol.hpp

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

/** @file protocol.hpp
* @brief Network communication protocol.
*
* This file contains all of the networking logic.
* The main class is nms::protocol::NMSProtocol. Use this file within an
* Application Control entitiy.
* This file depends only on the file "notifications.hpp" that contains the
* types for the interface to the Application Control entitiy.
*
* @author Alexander Korsunsky
*/

/** @defgroup proto Communication Protocol */



#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <stdexcept>
#include <string>

#include <boost/thread/thread.hpp>
#include <boost/statechart/asynchronous_state_machine.hpp>

#include "protocol/errors.hpp"
#include "control/notifications.hpp"



namespace nms
{
namespace protocol
{




/** Client Communication Protocol.
* @ingroup proto
* Use this class to create a black box that handles all the network stuff for
* you.
* Request are handled by the public functions connect_to, send and disconnect.
* Replies will be dispatched to the callback function you supply in the constructor.
*/
class NMSProtocol
{
    /** An own thread for the State Machine*/
    boost::thread machine_thread;

    /** The scheduler for the asynchronous state machine */
    boost::statechart::fifo_scheduler<> machine_scheduler;

    /** The event processor handle for the state machine */
    boost::statechart::fifo_scheduler<>::processor_handle event_processor;

    /** The function object that will be called, if an event occurs.*/
    control::notif_callback_t notification_callback;

    /** How long to wait for the thread to join */
    enum { threadwait_ms = 3000 };
public:

    /** Constructor.
    * Creates a thread and initializes the Network machine.
    * @param _notification_callback The callback function where the events will
    * be dispatched
    */
    NMSProtocol(const control::notif_callback_t _notification_callback) throw();

    /** Destructor.
    * Stops the Network machine and destroys the thread.
    */
    ~NMSProtocol();

    /** Connect to a remote site.
     * @param id The string representation of the address of the remote site
    * @throws std::runtime_error if a ressource could not be allocated.
    * e.g. a threading resource.
    * @throws ProtocolError if a networking error occured
     */
    void connect_to(const std::wstring& id)
        throw(std::runtime_error, ProtocolError);



    /** Send message to connected remote site.
     * @param msg The message you want to send
    * @throws std::runtime_error if a ressource could not be allocated.
    * e.g. a threading resource.
    * @throws ProtocolError if a networking error occured
     */
    void send(const std::wstring& msg)
        throw(std::runtime_error, ProtocolError);


    /** Disconnect from the remote site.
    * @throws std::runtime_error if a ressource could not be allocated.
    * e.g. a threading resource.
    * @throws ProtocolError if a networking error occured
    */
    void disconnect()
        throw(std::runtime_error, ProtocolError);

};

} // namespace protocol
} // namespace nms

#endif /*PROTOCOL_HPP_*/
