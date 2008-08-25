// statemachine.hpp

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

#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include <boost/shared_ptr.hpp>

#include <boost/statechart/asynchronous_state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "protocol/connection.hpp"


namespace nms
{
namespace protocol
{


/** @todo Insert documenation into the doc of the state reactions */

/** Template for events with one parameter.
* @ingroup proto_machine
* This class is to be used for boost::state_machine, to identify events that
* carry one parameter. It has tags to distinguish the event types.
*
* @tparam ParmType The type of the parameter
* @tparam EventTag The tag to distinguish the events. This can be an arbitary,
* but distinct type, such as empty structs.
*/
template <typename ParmType, typename EventTag>
struct Parm1Event
    : public boost::statechart::event<Parm1Event<ParmType, EventTag> >
{
    /** Parameter. */
    ParmType parm;

    /** Constructor. Initializes parameter */
    Parm1Event(const ParmType& _parm)
        : parm(_parm)
    {}
};

# if 0

/** Template for events with two parameters.
* @ingroup proto_machine
* This class is to be used for boost::state_machine, to identify events that
* carry one parameter. It has tags to distinguish the event types.
*
* @tparam ParmType The type of the parameter
* @tparam EventTag The tag to distinguish the events. This can be an arbitary,
* but distinct type, such as empty structs.
*/
template <typename Parm1Type, typename Parm2Type, typename EventTag>
struct Parm2Event
    : public boost::statechart::event<Parm2Event<Parm1Type,Parm2Type,EventTag> >
{
    /** First parameter. */
    Parm1Type parm1;

    /** Second parameter. */
    Parm2Type parm2;

    /** Constructor. Initializes parameter */
    Parm2Event(const Parm1Type& _parm1, const Parm2Type& _parm2)
        : parm1(_parm1), parm2(_parm2)
    {}
};


#endif


// Tags for different events with one parameter
/** Tag for connection requests. @ingroup proto_machine */
struct EventConnectRequestTag {};
/** Tag for connection reports. @ingroup proto_machine */
struct EventConnectReportTag {};
/** Tag for received messages. @ingroup proto_machine */
struct EventRcvdMsgTag {};
/** Tag for sent messages. @ingroup proto_machine */
struct EventSendMsgTag {};
/** Tag for disconnections. @ingroup proto_machine */
struct EventDisconnectedTag {};

// Typedefs for events with one parameter.

/** Event for connection requests.  @ingroup proto_machine*/
typedef Parm1Event<std::wstring, EventConnectRequestTag> EventConnectRequest;
/** Event for connection reports. @ingroup proto_machine */
typedef Parm1Event<std::wstring, EventConnectReportTag> EventConnectReport;
/** Event for received messages. @ingroup proto_machine */
typedef Parm1Event<std::wstring, EventRcvdMsgTag> EventRcvdMsg;
/** Event for sent messages. @ingroup proto_machine */
typedef Parm1Event<std::wstring, EventSendMsgTag> EventSendMsg;
/** Event for disconnections. @ingroup proto_machine */
typedef Parm1Event<std::wstring, EventDisconnectedTag> EventDisconnected;

/** Event for disconnect requests. @ingroup proto_machine */
struct EventDisconnectRequest : boost::statechart::event<EventDisconnectRequest>
{};


// forward declarations for the main protocol machines
struct StateUnconnected;

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
struct ProtocolMachine
    : public boost::statechart::asynchronous_state_machine<
            ProtocolMachine,
            StateUnconnected
        >
{
    /** The callback for notifications. */
    control::notif_callback_t notification_callback;

    /** A pointer to a NMSConnection object that is valid when in state
    TryingToConnect and Connected */
    boost::shared_ptr<NMSConnection> connection;

    /** Constructor. To be used only by Boost.Statechart classes. */
    ProtocolMachine(my_context ctx,
                    const control::notif_callback_t& _notification_callback)
        throw()
        : my_base(ctx),
            notification_callback(_notification_callback)
    {}

    /** */
    static void notificationTranslator (
        outermost_context_type& _this,
        const control::ProtocolNotification& notification
    ) throw();
};

// forward declarations that are needed for transition reactions.
struct StateIdle;
struct StateTryingConnect;

/** If the Protocol is unconnected.
* @ingroup proto_machine
* Reacting to:
* Nothing. (defer to substates)
*/
struct StateUnconnected
    : public boost::statechart::simple_state<StateUnconnected,
                                                ProtocolMachine,
                                                StateIdle>
{ };

/** If the protocol is in a connected state.
* @ingroup proto_machine
*
* Reacting to:
* EventSendMsg,
* EventRcvdMsg,
* EventDisconnectRequest
*/
struct StateConnected
    : public boost::statechart::simple_state<StateConnected,
                                                ProtocolMachine>
{
    /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EventSendMsg >,
        boost::statechart::custom_reaction< EventRcvdMsg >,
        boost::statechart::custom_reaction< EventDisconnected >,
        boost::statechart::custom_reaction< EventDisconnectRequest >
    > reactions;

    StateConnected();

    /** React to a EventSendMsg Event.
    *
    */
    boost::statechart::result react(const EventSendMsg& msg);

    /** React to a EventRcvdMsg Event.
    *
    */
    boost::statechart::result react(const EventRcvdMsg& msg);

    /** React to a EventDisconnected Event.
    *
    */
    boost::statechart::result react(const EventDisconnected& evt);

    /** React to a EventDisconnected Event.
    *
    */
    boost::statechart::result react(const EventDisconnectRequest&);

};



/** If the protocol is doing nothing.
* @ingroup proto_machine
* Substate of StateUnconnected.
*
* Reacting to:
* EventConnectReport,
* EventDisconnectRequest
*/
struct StateTryingConnect
    : public boost::statechart::simple_state<StateTryingConnect,
                                                StateUnconnected>
{
    /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EventConnectReport >,
        boost::statechart::custom_reaction< EventDisconnectRequest >,
        boost::statechart::custom_reaction <EventDisconnected >
    > reactions;

    /** React to a EventConnectReport Event.
    *
    */
    boost::statechart::result react(const EventConnectReport& rprt);

    /** React to a EventDisconnected Event.
    *
    */
    boost::statechart::result react(const EventDisconnected& evt);

    /** React to a EventDisconnected Event.
    *
    */
    boost::statechart::result react(const EventDisconnectRequest&);
};




/** If the protocol is doing nothing.
* @ingroup proto_machine
* Substate of StateUnconnected.
*
* Reacting to:
* EventConnectRequest
*/
struct StateIdle
    : public boost::statechart::simple_state<StateIdle,
                                                StateUnconnected>
{
    /** State reactions. */
    typedef boost::statechart::custom_reaction< EventConnectRequest > reactions;

    /** React to a EventConnectRequest Event.
    *
    */
    boost::statechart::result react(const EventConnectRequest& request);
};




} // namespace protocol
} // namespace nms


#endif // define STATEMACHINE_HPP
