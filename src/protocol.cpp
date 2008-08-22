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

#include <vector>

#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>


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


using boost::asio::ip::tcp;


/** This is a class that represents a connection to a remote peer.
* @ingroup proto
*
*/
class NMSConnection
{
    // wait for the thread 3 seconds
    enum { threadwait_ms = 3000 };


    /** The working thread */
    boost::thread worker;

    /** The IO service object */
    boost::asio::io_service io_service;

    /** The socket for the connection */
    tcp::socket socket;

    /** The callback for notifications. */
    control::notif_callback_t notification_callback;

    void resolveHandler(const boost::system::error_code& error,
                    tcp::resolver::iterator endpoint_iterator,
                    const std::wstring& id
                    );

    void connectHandler(const boost::system::error_code& error);

    void sendHandler(const boost::system::error_code& error,
                     std::size_t bytes_transferred,
                     unsigned char* sendbuf);

public:

    NMSConnection(const std::wstring& where,
                    const control::notif_callback_t& _notification_callback)
        throw();

    ~NMSConnection()
    {
        disconnect();
    }

    void send(const std::wstring& msg);


    void disconnect() throw();
};


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
        boost::statechart::custom_reaction<EventDisconnected>,
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

        outermost_context().connection->send(msg.parm);

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

    /** React to a EventDisconnected Event.
    *
    */
    boost::statechart::result react(const EventDisconnected& evt)
    {
        context<ProtocolMachine>()
            .notification_callback(
                    control::DisconnectedNotification(evt.parm)
                );

        // delete the Connection object, therefore closing the connection
        // and terminating the thread
        outermost_context().connection.reset();

        return transit<StateUnconnected>();
    }

    /** React to a EventDisconnected Event.
    *
    */
    boost::statechart::result react(const EventDisconnectRequest&)
    {

        //ask the Connection object to disconnect itself
        outermost_context().connection->disconnect();
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
        boost::statechart::custom_reaction< EventDisconnectRequest >,
        boost::statechart::custom_reaction<EventDisconnected>
    > reactions;

    /** React to a EventConnectReport Event.
    *
    */
    boost::statechart::result react(const EventConnectReport& rprt)
    {
        if (rprt.parm.empty())
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
                        (L"Sorry, connection failed: " + rprt.parm)
                    );

            // delete the Connection object, therefore closing the connection
            // and terminating the thread
            outermost_context().connection.reset();

            return transit<StateUnconnected>();
        }
    }

    /** React to a EventDisconnected Event.
    *
    */
    boost::statechart::result react(const EventDisconnected& evt)
    {
        context<ProtocolMachine>()
            .notification_callback(
                    control::DisconnectedNotification(evt.parm)
                );

        // delete the Connection object, therefore closing the connection
        // and terminating the thread
        outermost_context().connection.reset();

        return transit<StateUnconnected>();
    }

    /** React to a EventDisconnected Event.
    *
    */
    boost::statechart::result react(const EventDisconnectRequest&)
    {

        //ask the Connection object to disconnect itself
        outermost_context().connection->disconnect();

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
        // create an object of type NMSConnection
        outermost_context().connection.reset(
            new NMSConnection(
                request.parm,
                boost::bind(&ProtocolMachine::notificationTranslator,
                    boost::ref(outermost_context()),
                    _1
                )
            )
        );


        return transit<StateTryingConnect>();
    }
};






NMSConnection::NMSConnection(const std::wstring& where,
                             const control::notif_callback_t&
                                _notification_callback)
    throw()
    : notification_callback(_notification_callback),
      socket(io_service)
{


    try {
        // get ourself a tokenizer
        typedef boost::tokenizer<boost::char_separator<wchar_t>,
                                std::wstring::const_iterator, std::wstring>
            tokenizer;

        // get the part before the colon and the part after the colon
        boost::char_separator<wchar_t> colons(L":");
        tokenizer tokens(where, colons);

        tokenizer::iterator tok_iter = tokens.begin();

        // bail out, if there were no tokens
        if ( tok_iter == tokens.end() )
            throw ProtocolError("Invalid remote site identifier.");

        // create host from first token
        std::string host(tok_iter->begin(), tok_iter->end());

        if ( ++tok_iter == tokens.end() )
            throw ProtocolError("Invalid remote site identifier.");

        // create service from second token
        std::string service(tok_iter->begin(), tok_iter->end());

        // bail out if there is another colon
        if ( ++tok_iter != tokens.end() )
            throw ProtocolError("Invalid remote site identifier.");





        tcp::resolver resolver(io_service); // get a resolver
        tcp::resolver::query query(host, service); // create a query

        // dispatch an asynchronous resolve request
        resolver.async_resolve(
            query,
            boost::bind(
                    &NMSConnection::resolveHandler, this,
                    _1 , _2,
                    where
                )
            );

        // start a new thread that processes all asynchronous operations
        worker = boost::thread(
                    boost::bind(&boost::asio::io_service::run, &io_service)
                    );

    }
    catch (const std::exception& e)
    {
        std::string errmsg(e.what());

        notification_callback(
            control::ReportNotification<
                    control::ProtocolNotification::ID_CONNECT_REPORT
                    >
                (std::wstring(errmsg.begin(), errmsg.end()))
            );
    }

}


void NMSConnection::resolveHandler(const boost::system::error_code& error,
                        tcp::resolver::iterator endpoint_iterator,
                        const std::wstring& id
                       )
{


    // if there was an error, report it
    if (error)
    {
        std::string errmsg(error.message());

        notification_callback(
            control::ReportNotification<
                    control::ProtocolNotification::ID_CONNECT_REPORT
                    >
                (std::wstring(errmsg.begin(), errmsg.end()))
            );
    }


    // otherwise try to connect


    // dispatch an asynchronous connect request
    socket.async_connect(
        *endpoint_iterator,
        boost::bind(&NMSConnection::connectHandler, this, _1)
        );

}




void NMSConnection::connectHandler(const boost::system::error_code& error)
{
    if (error)
    {
        std::string errmsg(error.message());

        // report failure
        notification_callback(
            control::ReportNotification<
                    control::ProtocolNotification::ID_CONNECT_REPORT
                    >
                (std::wstring(errmsg.begin(), errmsg.end()))
            );
    }

    notification_callback(
    control::ReportNotification<
            control::ProtocolNotification::ID_CONNECT_REPORT
            >
        ()
    );
}


void NMSConnection::sendHandler(const boost::system::error_code& /* error */,
    std::size_t /* bytes_transferred */,
    unsigned char* sendbuf
)
{
    // in any case delete the sendbuffer
    delete[] sendbuf;

    // if there was an error, close the socket
    disconnect();
}

void NMSConnection::send(const std::wstring& msg)
{
    std::size_t sendbuf_size = msg.size()*sizeof(std::wstring::value_type);

    unsigned char* sendbuf = new unsigned char[sendbuf_size];

    std::copy(msg.begin(), msg.end(),
        reinterpret_cast<std::wstring::value_type*>(sendbuf)
    );

    // send the bytes
    async_write(socket,
        boost::asio::buffer(sendbuf, sendbuf_size),
        boost::bind(&NMSConnection::sendHandler, this, _1, _2, sendbuf)
    );
};

void NMSConnection::disconnect()
    throw()
{
    boost::system::error_code ec;

    // shutdown receiver and sender end of the socket, ignore errors
    socket.shutdown(tcp::socket::shutdown_both, ec);

    // stop the io_service object
    io_service.stop();

    // try to join the thread
    try {
        worker.timed_join(boost::posix_time::millisec(threadwait_ms));

    }
    catch(...)
    {
    }

    // if the thread finished, return. otherwise try to kill the thread
    if (worker.get_id() == boost::thread::id())
        return;

    // try to interrrupt the thread
    worker.interrupt();

    // when the thread object goes out of scope, the thread detaches

    // finally notify the caller about the disconnection
    notification_callback(
        control::ProtocolNotification(
            control::ProtocolNotification::ID_DISCONNECTED
        )
    );

}



#if 1

void ProtocolMachine::notificationTranslator (
    outermost_context_type& _this,
    const control::ProtocolNotification& notification
) throw()
{
    using namespace nms::control;

    switch (notification.id)
    {
        case ProtocolNotification::ID_DISCONNECTED:
        {
            // cast to a Disconnected Notification
            const DisconnectedNotification& notif =
                static_cast<const DisconnectedNotification&>(notification);

            // post the event to the State Machine
            _this.post_event(EventDisconnected(notif.msg));

            break;
        }

        case ProtocolNotification::ID_RECEIVED_MSG:
        {
            const ReceivedMsgNotification& notif =
                static_cast<const ReceivedMsgNotification&> (notification);

            // post the event to the State Machine
            _this.post_event(EventRcvdMsg(notif.msg));


            break;
        }


        case ProtocolNotification::ID_CONNECT_REPORT:
        {
            const RequestReport& rprt =
                static_cast<
                    const ReportNotification<
                                ProtocolNotification::ID_CONNECT_REPORT>&
                            >
                            (notification);

            _this.post_event(EventConnectReport(rprt.failure_reason));

            break;
        }


        default:
            /* nothing */;

    }


}
#endif


NMSProtocol::NMSProtocol(const control::notif_callback_t _notification_callback)
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
    machine_thread = boost::thread(
        boost::bind(
            &boost::statechart::fifo_scheduler<>::operator(),
            &machine_scheduler,
            0
            )
        );
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
    boost::intrusive_ptr<EventConnectRequest>
    connect_request(new EventConnectRequest(id));

    machine_scheduler.queue_event(event_processor, connect_request);
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
    boost::intrusive_ptr<EventDisconnectRequest>
    disconnect_evt(new EventDisconnectRequest);

    machine_scheduler.queue_event(event_processor, disconnect_evt);
}


