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


using namespace nms;
using namespace protocol;



struct EventConnectRequest : boost::statechart::event<EventConnectRequest>
{
    std::wstring where;

    EventConnectRequest(const std::wstring& _where)
        : where(_where)
    {}
};

struct EventConnectReport : boost::statechart::event<EventConnectRequest>
{
    bool success;

    EventConnectReport(bool _success)
        : success(_success)
    {}
};

struct EventRcvdMsg : boost::statechart::event<EventRcvdMsg>
{
    std::wstring msg;

    EventRcvdMsg(const std::wstring& _msg)
        : msg(_msg)
    { }
};


struct EventSendMsg : boost::statechart::event<EventSendMsg>
{
    std::wstring msg;

    EventSendMsg(const std::wstring& _msg)
        : msg(_msg)
    { }
};

struct EventDisconnect : boost::statechart::event<EventDisconnect>
{};




struct StateUnconnected;
struct StateConnected;


struct ProtocolMachine
    : public boost::statechart::asynchronous_state_machine<
        ProtocolMachine,
        StateUnconnected
        >
{
    boost::function1 <void, const control::ProtocolNotification&>
        notification_callback;

    ProtocolMachine(my_context ctx,
                    const boost::function1
                        <void, const control::ProtocolNotification&>&
                        _notification_callback) throw()
        : my_base(ctx),
            notification_callback(_notification_callback)
    {}
};





struct StateIdle;
struct StateTryingConnect;

struct StateUnconnected
    : public boost::statechart::simple_state<StateUnconnected,
                                                ProtocolMachine,
                                                StateIdle>
{ };


struct StateIdle
    : public boost::statechart::simple_state<StateIdle,
                                                StateUnconnected>
{
    typedef boost::statechart::custom_reaction< EventConnectRequest > reactions;

    boost::statechart::result react(const EventConnectRequest& request)
    {
        std::cout<<"Trying to connect to "<<std::string(request.where.begin(), request.where.end())<<"\n";
        return transit<StateTryingConnect>();
    }
};

struct StateTryingConnect
    : public boost::statechart::simple_state<StateTryingConnect,
                                                StateUnconnected>
{
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EventConnectReport >,
        boost::statechart::custom_reaction<EventDisconnect>
    > reactions;

    boost::statechart::result react(const EventConnectReport& rprt)
    {
        if (rprt.success)
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

    boost::statechart::result react(const EventDisconnect&)
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ProtocolNotification
                    (control::ProtocolNotification::ID_DISCONNECTED)
                );

        return transit<StateUnconnected>();
    }

};


struct StateConnected
    : public boost::statechart::simple_state<StateConnected,
                                                ProtocolMachine>
{
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EventSendMsg >,
        boost::statechart::custom_reaction< EventRcvdMsg >,
        boost::statechart::custom_reaction<EventDisconnect>
    > reactions;

    StateConnected()
    {
        std::cout<<"We are connected!\n";
    }


    boost::statechart::result react(const EventSendMsg& msg)
    {
        std::cout<<"Sending message: ";
        std::cout<<std::string(msg.msg.begin(), msg.msg.end())<<'\n';

        return discard_event();
    }

    boost::statechart::result react(const EventRcvdMsg& msg)
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ReceivedMsgNotification
                    (msg.msg)
                );

        return discard_event();
    }

    boost::statechart::result react(const EventDisconnect&)
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
    boost::intrusive_ptr<EventDisconnect>
    disconnect_evt(new EventDisconnect);

    machine_scheduler.queue_event(event_processor, disconnect_evt);
#endif
}



