// protocol.cpp

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

#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>

#include <boost/statechart/asynchronous_state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "protocol.hpp"

/** @defgroup proto_machine Comunication Protocol State Machine
* @ingroup proto */

using namespace nms;
using namespace protocol;



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

// Tags for different events with one parameter
/** Tag for connection requests. @ingroup proto_machine */
struct EventConnectRequestTag {};
/** Tag for connection reports. @ingroup proto_machine */
struct EventConnectReportTag {};
/** Tag for received messages. @ingroup proto_machine */
struct EventRcvdMsgTag {};
/** Tag for sent messages. @ingroup proto_machine */
struct EventSendMsgTag {};

// Typedefs for events with one parameter.

/** Event for connection requests.  @ingroup proto_machine*/
typedef Parm1Event<std::wstring, EventConnectRequestTag> EventConnectRequest;
/** Event for connection reports. @ingroup proto_machine */
typedef Parm1Event<bool, EventConnectReportTag> EventConnectReport;
/** Event for received messages. @ingroup proto_machine */
typedef Parm1Event<std::wstring, EventRcvdMsgTag> EventRcvdMsg;
/** Event for sent messages. @ingroup proto_machine */
typedef Parm1Event<std::wstring, EventSendMsgTag> EventSendMsg;

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
    boost::function1 <void, const control::ProtocolNotification&>
        notification_callback;

    /** Constructor. To be used only by Boost.Statechart classes. */
    ProtocolMachine(my_context ctx,
                    const boost::function1
                        <void, const control::ProtocolNotification&>&
                        _notification_callback)
        throw()
        : my_base(ctx),
            notification_callback(_notification_callback)
    {}
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
        boost::statechart::custom_reaction<EventDisconnectRequest>
    > reactions;

    StateConnected()
    {
        std::cout<<"We are connected!\n";
    }

    /** React to a EventSendMsg Event.
    *
    */
    boost::statechart::result react(const EventSendMsg& msg)
    {
        std::cout<<"Sending message: ";
        std::cout<<std::string(msg.parm.begin(), msg.parm.end())<<'\n';

        return discard_event();
    }

    /** React to a EventRcvdMsg Event.
    *
    */
    boost::statechart::result react(const EventRcvdMsg& msg)
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ReceivedMsgNotification
                    (msg.parm)
                );

        return discard_event();
    }

    /** React to a EventDisconnectRequest Event.
    *
    */
    boost::statechart::result react(const EventDisconnectRequest&)
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ProtocolNotification
                    (control::ProtocolNotification::ID_DISCONNECTED)
                );

        return transit<StateUnconnected>();
    }

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
    boost::statechart::result react(const EventConnectRequest& request)
    {
        std::cout<<"Trying to connect to "<<std::string(request.parm.begin(), request.parm.end())<<"\n";
        return transit<StateTryingConnect>();
    }
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
        boost::statechart::custom_reaction<EventDisconnectRequest>
    > reactions;

    /** React to a EventConnectReport Event.
    *
    */
    boost::statechart::result react(const EventConnectReport& rprt)
    {
        if (rprt.parm)
        {
            context<ProtocolMachine>()
                .notification_callback(
                    control::ReportNotification
                        <control::ProtocolNotification::ID_CONNECT_REPORT>()
                    );

            return transit<StateConnected>();
        }
        else
        {
            context<ProtocolMachine>()
                .notification_callback(
                    control::ReportNotification
                        <control::ProtocolNotification::ID_CONNECT_REPORT>
                        (L"Sorry, connection failed.\n")
                    );

            return transit<StateUnconnected>();
        }
    }

    /** React to a EventDisconnectRequest Event.
    *
    */
    boost::statechart::result react(const EventDisconnectRequest&)
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ProtocolNotification
                    (control::ProtocolNotification::ID_DISCONNECTED)
                );

        return transit<StateUnconnected>();
    }

};



NMSProtocol::NMSProtocol(const boost::function1<void, const control::ProtocolNotification&>&
                            _notification_callback)
            throw()

    : notification_callback(_notification_callback),
        machine_scheduler(true)
{
    // create an event processor for our state machine
    event_processor =
        machine_scheduler.create_processor<ProtocolMachine>(
            _notification_callback
            );

    // initiate the event processor
    machine_scheduler.initiate_processor(event_processor);

    // machine_thread is in state "not-a-thread", so we will use the move
    // semantics provided by boost::thread to create a new thread and assign it
    // the variable
#if 1
    machine_thread = boost::thread(
        boost::bind(
            &boost::statechart::fifo_scheduler<>::operator(),
            &machine_scheduler,
            0
            )
        );
#endif

}

NMSProtocol::~NMSProtocol()
{
    // stop the network machine
    machine_scheduler.terminate();

    try {
        // give the thread a few seconds time to join
        machine_thread.timed_join(boost::posix_time::millisec(threadwait_ms));
    }
    catch(...)
    {}

    // if the thread finished, return. otherwise try to kill the thread
    if (machine_thread.get_id() == boost::thread::id())
        return;

    machine_thread.interrupt();

    // if interruption failed, let the thread detach
}


void NMSProtocol::connect_to(const std::wstring& id)
    throw(std::runtime_error, ProtocolError)
{
#if 1
    boost::intrusive_ptr<EventConnectRequest>
    connect_request(new EventConnectRequest(id));

    boost::intrusive_ptr<EventConnectReport>
    connect_reply(new EventConnectReport(true));


    machine_scheduler.queue_event(event_processor, connect_request);
    machine_scheduler.queue_event(event_processor, connect_reply);
#endif
}



void NMSProtocol::send(const std::wstring& msg)
    throw(std::runtime_error, ProtocolError)
{
    boost::intrusive_ptr<EventSendMsg>
    send_evt(new EventSendMsg(msg));

    machine_scheduler.queue_event(event_processor, send_evt);
}


void NMSProtocol::disconnect()
    throw(std::runtime_error, ProtocolError)
{
#if 1
    boost::intrusive_ptr<EventDisconnectRequest>
    disconnect_evt(new EventDisconnectRequest);

    machine_scheduler.queue_event(event_processor, disconnect_evt);
#endif
}



