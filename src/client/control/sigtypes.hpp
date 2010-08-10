// sigtypes.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2010  Alexander Korsunsky
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


#ifndef SIGTYPES_HPP
#define SIGTYPES_HPP

#include <boost/shared_ptr.hpp>
#include "bytes.hpp"

namespace nuke_ms
{
namespace control
{


/** Identification of the server location */
struct ServerLocation
{
    typedef boost::shared_ptr<ServerLocation> ptr_t;
    typedef boost::shared_ptr<const ServerLocation> const_ptr_t;

    byte_traits::string where; /**< String with hostname or ip address */
};

/** Message that is sent or received over the network.
*/
struct Message
{
    typedef boost::shared_ptr<Message> ptr_t;
    typedef boost::shared_ptr<const Message> const_ptr_t;

    /** Type for unique message ID */
    typedef unsigned short message_id_t;

    message_id_t id; /**< Unique message ID */
    byte_traits::string str; /**< Message string */
};

/** Status report of connection state changes
*/
struct ConnectionStatusReport
{
    typedef boost::shared_ptr<ConnectionStatusReport> ptr_t;
    typedef boost::shared_ptr<const ConnectionStatusReport> const_ptr_t;

    /** Type for the current connection state */
    enum connect_state_t
    {
        CNST_DISCONNECTED,
        CNST_CONNECTING,
        CNST_CONNECTED
    };

    /** Type for the reason of a state change */
    enum statechange_reason_t
    {
        STCHR_NO_REASON = 0, /**<No reason, used for example after state query*/
        STCHR_CONNECT_SUCCESSFUL, /**< Connection attempt was successful */
        STCHR_CONNECT_FAILED, /**< Connection attempt failed */
        STCHR_SOCKET_CLOSED, /**< Connection to remote server lost */
        STCHR_USER_REQUESTED /**< User requested state change */
    };

    connect_state_t newstate; /**< current connection state */
    statechange_reason_t statechange_reason; /**< reason for state change */
    byte_traits::string msg; /**< Optional message describing the reason */
};


/** Report for sent messages
*/
struct SSendReport
{
    typedef boost::shared_ptr<SSendReport> ptr_t;
    typedef boost::shared_ptr<const SSendReport> const_ptr_t;

    /** Type for the report of the sent message.
    * All failure reports are guaranteed to have the first bit set to 0,
    * all success reports are guaranteed to have the first bit set to 1
    */
    enum send_state_t
    {
        SS_SEND_NAK = 0, /**< Failed to send message */
        SS_SEND_ACK = 1 /**< Message sent on our end */
        //SS_SERV_NAK = 2, /**< Server did not receive message */
        //SS_SERV_ACK = 3, /**< Server received message */
        //SS_USER_NAK = 6, /**< Remote user did not receive message */
        //SS_USER_ACK = 7  /**< Remote user received message */
    };

    Message::message_id_t message_id; /**< ID of the message in question */
};


// Signals issued by the GUI
typedef boost::signals2::signal<void (control::ServerLocation::const_ptr_t)>
    SignalConnectTo;
typedef boost::signals2::signal<void (control::Message::const_ptr_t)>
    SignalSendMessage;
typedef boost::signals2::signal<void ()> SignalConnectionStatusQuery;
typedef boost::signals2::signal<void ()> SignalDisconnect;
typedef boost::signals2::signal<void ()> SignalExitApp;



// Signals issued by the Protocol
typedef boost::signals2::signal<void (control::Message::const_ptr_t)>
    SignalRcvMessage;
typedef boost::signals2::signal<
    void (const control::ConnectionStatusReport::const_ptr_t)>
    SignalConnectionStatusReport;
typedef boost::signals2::signal<
    void (control::SSendReport::const_ptr_t)>
    SignalSendReport;





} // namespace control
} // namespace nuke_ms


#endif // ifndef SIGTYPES_HPP


