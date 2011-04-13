// sigtypes.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2010  Alexander Korsunsky
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

/** @file clientnode/sigtypes.hpp
* @brief Signals issued by the ClientNode module
* @ingroup clientnode
*
* Contains the types and function declarations needed by the signals issued by
* ClientNode.
*
* @author Alexander Korsunsky
*/


#ifndef SIGTYPES_HPP
#define SIGTYPES_HPP

#include <boost/signals2/signal.hpp>
#include <boost/shared_ptr.hpp>

#include "bytes.hpp"
#include "neartypes.hpp"

namespace nuke_ms
{


/** @addtogroup clientnode Communication Protocol 
 * @{
*/
	

namespace clientnode
{


/** Identification of the server location */
struct ServerLocation
{
    typedef boost::shared_ptr<ServerLocation> ptr_t;
    typedef boost::shared_ptr<const ServerLocation> const_ptr_t;

    byte_traits::native_string where; /**< String with hostname or ip address */
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
        STCHR_INTERNAL_ERROR, /**< Internal error has occured, refer to msg */
        STCHR_CONNECT_SUCCESSFUL, /**< Connection attempt was successful */
        STCHR_CONNECT_FAILED, /**< Connection attempt failed */
        STCHR_SOCKET_CLOSED, /**< Connection to remote server lost */
        STCHR_USER_REQUESTED, /**< User requested state change */
        STCHR_BUSY /**< An operation is currently being performed */
    };

    connect_state_t newstate; /**< current connection state */
    statechange_reason_t statechange_reason; /**< reason for state change */
    byte_traits::native_string msg;/**< Optional message describing the reason*/
};


/** Report for sent messages
*/
struct SendReport
{
    typedef boost::shared_ptr<SendReport> ptr_t;
    typedef boost::shared_ptr<const SendReport> const_ptr_t;


    NearUserMessage::msg_id_t message_id; /**< ID of the message in question */
    bool send_state; /**< Was it sent or not */

	/** Enum for the reasons for failure while sending a message */
    enum send_rprt_reason_t
    {
        SR_SEND_OK, /**< All cool. */
        SR_SERVER_NOT_CONNECTED, /**< Not connected to server */
        SR_CONNECTION_ERROR /**< Network failure */
    } reason; /**< Reason for failure while sending a message */
	
	/** Reason for whatever this report is about */
    byte_traits::native_string reason_str; 
};


// Signals issued by the Protocol

/** Signal for incoming messages*/
typedef boost::signals2::signal<void (NearUserMessage::const_ptr_t)>
    SignalRcvMessage;
	
/** signal for connection status reports */
typedef boost::signals2::signal<
    void (ConnectionStatusReport::const_ptr_t)>
    SignalConnectionStatusReport;

/** signal for send reports */
typedef boost::signals2::signal<
    void (SendReport::const_ptr_t)>
    SignalSendReport;

} // namespace control

/**@}*/ // addtogroup clientnode

} // namespace nuke_ms


#endif // ifndef SIGTYPES_HPP


