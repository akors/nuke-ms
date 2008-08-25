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

#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <boost/shared_ptr.hpp>

#include <boost/statechart/asynchronous_state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "control/notifications.hpp"

namespace nms
{
namespace protocol
{

void catchThread(boost::thread& thread, unsigned threadwait_ms)
    throw();

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
typedef Parm1Event<std::wstring, EventConnectRequestTag>
    EventConnectRequest;
/** Event for connection reports. @ingroup proto_machine */
typedef Parm2Event<
    std::wstring,
    boost::shared_ptr<boost::asio::ip::tcp::socket>,
    EventConnectReportTag
>
    EventConnectReport;
/** Event for received messages. @ingroup proto_machine */
typedef Parm1Event<std::wstring, EventRcvdMsgTag>
    EventRcvdMsg;
/** Event for sent messages. @ingroup proto_machine */
typedef Parm1Event<std::wstring, EventSendMsgTag>
    EventSendMsg;
/** Event for disconnections. @ingroup proto_machine */
typedef Parm1Event<std::wstring, EventDisconnectedTag>
    EventDisconnected;

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

    /** The IO Service object used by the protocol machine */
    boost::asio::io_service io_service;

    /** The thread processing IO Operations.
     * "Not-a-thread" in StateUnconnected.
    */
    boost::thread worker;



    /** Constructor. To be used only by Boost.Statechart classes. */
    ProtocolMachine(my_context ctx,
                    const control::notif_callback_t& _notification_callback)
        throw()
        : my_base(ctx),
            notification_callback(_notification_callback)
    {}

    /** Pointer to a socket.
        * This must be set before entering the state StateConnected
    */
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

    /** A string that identifies where to connect
     * This must be set before entering the state StateTryingToConnect
    */
    std::wstring connect_where;
};


/** If the protocol  is unconnected.
* @ingroup proto_machine
* Substate of StateUnconnected.
*
* Reacting to:
* EventConnectRequest
*/
struct StateUnconnected
    : public boost::statechart::state<StateUnconnected,
                                                ProtocolMachine>
{
    /** State reactions. */
    typedef boost::statechart::custom_reaction< EventConnectRequest > reactions;

    /** Constructor.
     * Stops the worker thread in the Protocol Machine.
    */
    StateUnconnected( my_context ctx ) throw();


    /** React to a EventConnectRequest Event.
    *
    */
    boost::statechart::result react(const EventConnectRequest& request);
};


/** If the protocol is trying to connect to a host
* @ingroup proto_machine
* Substate of StateUnconnected.
*
* Reacting to:
* EventConnectReport,
* EventDisconnectRequest
*/
struct StateTryingConnect
    : public boost::statechart::state<StateTryingConnect,
                                                ProtocolMachine>
{

    /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EventConnectReport >,
        boost::statechart::custom_reaction< EventDisconnectRequest >,
        boost::statechart::custom_reaction <EventDisconnected >
    > reactions;


    StateTryingConnect( my_context ctx ) throw();


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

private:
    static void resolveHandler(
        const boost::system::error_code& error,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
        outermost_context_type& _outermost_context
    );

    static void connectHandler(
        const boost::system::error_code& error,
        boost::shared_ptr<boost::asio::ip::tcp::socket> socket,
        outermost_context_type& _outermost_context
    );

};


/** If the protocol is in a connected state.
* @ingroup proto_machine
*
* Reacting to:
* EventSendMsg,
* EventRcvdMsg,
* EventDisconnectRequest
*/
struct StateConnected
    : public boost::statechart::state<
        StateConnected,
        ProtocolMachine
    >
{
    /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EventSendMsg >,
        boost::statechart::custom_reaction< EventRcvdMsg >,
        boost::statechart::custom_reaction< EventDisconnected >,
        boost::statechart::custom_reaction< EventDisconnectRequest >
    > reactions;

    StateConnected(my_context ctx);

    ~StateConnected();


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

private:
    boost::shared_ptr<boost::asio::ip::tcp::socket>& socket_ptr;

    static void receiveHandler(
        const boost::system::error_code& error,
        std::size_t bytes_transferred,
        unsigned char* rcvbuf,
        outermost_context_type& _outermost_context
    );

    static void sendHandler(
        const boost::system::error_code& error,
        std::size_t bytes_transferred,
        unsigned char* sendbuf,
        outermost_context_type& _outermost_context
    );

};





} // namespace protocol
} // namespace nms


#endif // define STATEMACHINE_HPP
