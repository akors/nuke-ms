// statemachine.hpp

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


#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include <boost/asio.hpp>
#include <boost/statechart/asynchronous_state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/mpl/list.hpp>

#include "protocol/msglayer.hpp"
#include "control/notifications.hpp"

namespace nms
{
namespace protocol
{

// Event declarations

/** Event representing a Connection Request.
* @ingroup proto_machine
*/
struct EvtConnectRequest : public boost::statechart::event<EvtConnectRequest>
{
    /** Where to connect to. */
    std::wstring where;

    /** Constructor.
    * @param _where Where to connect to. */
    EvtConnectRequest(const std::wstring& _where)
        : where (_where)
    {}
};

/** Event informing about the outcome of an attempt to connect to a remote site.
* @ingroup proto_machine
*
*/
struct EvtConnectReport : public boost::statechart::event<EvtConnectReport>
{
    bool success; /**< true on success, false on failure */
    std::wstring message; /**< Message commenting the outcome. */

    /** Constructor. Initializes members.
    * @param _success true on success, false on failure
    * @param _message Message commenting the outcome.
    */
    EvtConnectReport(bool _success, const std::wstring& _message)
        : success(_success), message (_message)
    {}
};

/** Event representing a Disconnection Request.
* @ingroup proto_machine
*/
struct EvtDisconnectRequest :
    public boost::statechart::event<EvtDisconnectRequest>
{};

/** Event representing a sent message
* @ingroup proto_machine
*/
struct EvtSendMsg : public boost::statechart::event<EvtSendMsg>
{
    /** The text of the message. */
    std::wstring msg;

    /** Constructor.
    * @param _msg The text of the message.
    */
    EvtSendMsg(const std::wstring& _msg)
        : msg (_msg)
    {}
};


// Forward declaration of the Initial State
struct StateWaiting;


/** The Protoc State Machine.
* @ingroup proto_machine
* This class represents the Overall State of the Protocol.
* Events can be dispatched to this machine, and the according actions will be
* performed.
*
* Currently, this class is an "asynchronous_state_machine", which has it's own
* thread. Refer to the documention of Boost.Statechart for details on how to
* create and use this class, and how to dispatch events.
*/
struct ProtocolMachine :
    public boost::statechart::asynchronous_state_machine<
        ProtocolMachine,
        StateWaiting
    >
{
    /** Callback that will be used to inform the application */
    nms::control::notif_callback_t notification_callback;

    /** I/O Service Object */
    //boost::asio::io_service& io_service;

#if 0
    /** Constructor. To be used only by Boost.Statechart classes. */
    ProtocolMachine(my_context ctx,
                    nms::control::notif_callback_t _notification_callback,
                    boost::asio::io_service& _io_service);
#else
    /** Constructor. To be used only by Boost.Statechart classes. */
    ProtocolMachine(my_context ctx,
                    nms::control::notif_callback_t _notification_callback);
#endif
};



/** State indicating that the Protocol is Waiting to be connected.
* @ingroup proto_machine
*
* Reacting to:
* EvtConnectRequest
* EvtSendMsg
*/
struct StateWaiting :
    public boost::statechart::state<StateWaiting, ProtocolMachine>
{

    /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvtConnectRequest >,
        boost::statechart::custom_reaction<EvtSendMsg>
    > reactions;

    /** Constructor. To be used only by Boost.Statechart classes. */
    StateWaiting(my_context ctx);

    boost::statechart::result react(const EvtConnectRequest &);
    boost::statechart::result react(const EvtSendMsg &);

};


/** State indicating that the Connection is being established.
* @ingroup proto_machine
*
* Reacting to:
* EvtConnectReport
*/
struct StateNegotiating :
    public boost::statechart::state<StateNegotiating, ProtocolMachine>
{
     /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvtConnectReport >,
        boost::statechart::custom_reaction< EvtDisconnectRequest >,
        boost::statechart::custom_reaction< EvtSendMsg >
    > reactions;

    /** Constructor. To be used only by Boost.Statechart classes. */
    StateNegotiating(my_context ctx);

    boost::statechart::result react(const EvtConnectReport& evt);
    boost::statechart::result react(const EvtDisconnectRequest&);
    boost::statechart::result react(const EvtSendMsg& evt);
};


struct StateConnected :
    public boost::statechart::state<StateConnected, ProtocolMachine>
{
    /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<EvtDisconnectRequest>,
        boost::statechart::custom_reaction<EvtSendMsg>
    > reactions;

    /** Constructor. To be used only by Boost.Statechart classes. */
    StateConnected(my_context ctx);

    boost::statechart::result react(const EvtDisconnectRequest&);
    boost::statechart::result react(const EvtSendMsg& evt);
};


} // namespace protocol
} // namespace nms


#endif // ifndef STATEMACHINE_HPP