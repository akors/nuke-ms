// statemachine.cpp

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
#include <limits>
#include <algorithm>

#include "bytes.hpp"
#include "protocol/errors.hpp"
#include "protocol/statemachine.hpp"
#include "protocol/msglayer.hpp"

using namespace nms;
using namespace protocol;

using boost::asio::ip::tcp;





StateUnconnected::StateUnconnected( my_context ctx )
    throw ()
    : my_base(ctx)
{
    std::cout<<"Protocol unconnected.\n";

    // stop the service object if it's running
    outermost_context().io_service.stop();

    // stop the thread
    catchThread(outermost_context().worker, 3000u);

    // reset the io_service object
    outermost_context().io_service.reset();
}


boost::statechart::result
StateUnconnected::react(const EventConnectRequest& request)
{
    // set parameter for the StateTryingToConnect transition
    outermost_context().connect_where = request.parm;

    return transit<StateTryingConnect>();
}




StateTryingConnect::StateTryingConnect( my_context ctx )
    throw ()
    : my_base(ctx)
{
    try {
        // get ourself a tokenizer
        typedef boost::tokenizer<boost::char_separator<wchar_t>,
                                std::wstring::const_iterator, std::wstring>
            tokenizer;

        // get the part before the colon and the part after the colon
        boost::char_separator<wchar_t> colons(L":");
        tokenizer tokens(outermost_context().connect_where, colons);

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


        tcp::resolver resolver(outermost_context().io_service); //get a resolver
        tcp::resolver::query query(host, service); // create a query

        // dispatch an asynchronous resolve request
        resolver.async_resolve(
            query,
            boost::bind(
                &StateTryingConnect::resolveHandler,
                boost::asio::placeholders::error,
                boost::asio::placeholders::iterator,
                boost::ref(outermost_context())
            )
        );

        // start a new thread that processes all asynchronous operations
        outermost_context().worker = boost::thread(
            boost::bind(
                &boost::asio::io_service::run,
                &outermost_context().io_service
            )
        );

    }
    catch (const std::exception& e)
    {
        std::string errmsg(e.what());

        post_event(
            EventConnectReport(
                std::wstring(errmsg.begin(),errmsg.end()),
                boost::shared_ptr<tcp::socket>()
            )
        );
    }
}


void StateTryingConnect::resolveHandler(
    const boost::system::error_code& error,
    tcp::resolver::iterator endpoint_iterator,
    outermost_context_type& _outermost_context
)
{
    std::cout<<"resolveHandler invoked.\n";

    // if there was an error, report it
    if (endpoint_iterator == tcp::resolver::iterator() )
    {
        std::string errmsg;

        if (error)
            errmsg = error.message();
        else
            errmsg = "No hosts found.";

        _outermost_context.my_scheduler().queue_event(
            _outermost_context.my_handle(),
            boost::intrusive_ptr<EventConnectReport> (
                new EventConnectReport(
                    std::wstring(errmsg.begin(),errmsg.end()),
                    boost::shared_ptr<tcp::socket>()
                )
            )
        );

        return;
    }


    // otherwise try to connect

    boost::shared_ptr<tcp::socket>
    socket(new tcp::socket(_outermost_context.io_service));


    // dispatch an asynchronous connect request
    socket->async_connect(
        *endpoint_iterator,
        boost::bind(
            &StateTryingConnect::connectHandler,
            _1,
            socket,
            boost::ref(_outermost_context)
        )
    );
}


void StateTryingConnect::connectHandler(
    const boost::system::error_code& error,
    boost::shared_ptr<tcp::socket> socket,
    outermost_context_type& _outermost_context
)
{
    std::cout<<"connectHandler invoked.\n";

    boost::intrusive_ptr<EventConnectReport> evt_rprt;

    // if there was an error, create a negative reply
    if (error)
    {
        std::string errmsg(error.message());

        evt_rprt = new EventConnectReport(
            std::wstring(errmsg.begin(),errmsg.end()),
            boost::shared_ptr<tcp::socket>()
        );
    }
    else // if there was no error, create a positive reply
    {
        evt_rprt = new EventConnectReport(
            std::wstring(),
            socket
        );
    }

    _outermost_context.my_scheduler().
        queue_event(_outermost_context.my_handle(), evt_rprt);
}




boost::statechart::result
StateTryingConnect::react(const EventConnectReport& rprt)
{
    std::cout<<"Reacting to EventConnectReport.\n";

    if (rprt.parm2 && rprt.parm2->is_open())
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ReportNotification
                    <control::ProtocolNotification::ID_CONNECT_REPORT>()
                );

        outermost_context().socket_ptr = rprt.parm2;

        return transit<StateConnected>();
    }
    else
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ReportNotification
                    <control::ProtocolNotification::ID_CONNECT_REPORT>
                    (rprt.parm1)
                );

        // reset the pointer to the socket in the state machine,
        // since there should not be a socket, if the connection failed
        outermost_context().socket_ptr.reset();

        return transit<StateUnconnected>();
    }
}


boost::statechart::result
StateTryingConnect::react(const EventDisconnected& evt)
{
    context<ProtocolMachine>()
        .notification_callback(
                control::DisconnectedNotification(evt.parm)
            );

    return transit<StateUnconnected>();
}


boost::statechart::result
StateTryingConnect::react(const EventDisconnectRequest&)
{

    return transit<StateUnconnected>();
}











StateConnected::StateConnected( my_context ctx )
    : my_base(ctx), socket_ptr(outermost_context().socket_ptr)
{
    std::cout<<"We are connected!\n";

    //
}

StateConnected::~StateConnected()
{
    boost::system::error_code ec;

    // shutdown receiver and sender end of the socket, ignore errors
    socket_ptr->shutdown(tcp::socket::shutdown_both, ec);

    socket_ptr.reset();
}



boost::statechart::result StateConnected::react(const EventSendMsg& msg)
{
    std::cout<<"Sending message: ";
    std::cout<<std::string(msg.parm.begin(), msg.parm.end())<<'\n';

    // determine size of the message to be sent
    std::size_t sendbuf_size = msg.parm.size()*sizeof(std::wstring::value_type);

    // allocate a buffer for the message, and copy it over
    unsigned char* sendbuf = new unsigned char[sendbuf_size];

    std::copy(msg.parm.begin(), msg.parm.end(),
        reinterpret_cast<std::wstring::value_type*>(sendbuf)
    );

    // send the bytes
    async_write(*socket_ptr,
        boost::asio::buffer(sendbuf, sendbuf_size),
        boost::bind(
            &StateConnected::sendHandler,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            sendbuf, boost::ref(outermost_context()))
    );


    return discard_event();
}

boost::statechart::result StateConnected::react(const EventRcvdMsg& msg)
{
    context<ProtocolMachine>()
        .notification_callback(
            control::ReceivedMsgNotification
                (msg.parm)
            );

    return discard_event();
}


boost::statechart::result StateConnected::react(const EventDisconnected& evt)
{
    context<ProtocolMachine>()
        .notification_callback(
                control::DisconnectedNotification(evt.parm)
            );

    return transit<StateUnconnected>();
}


boost::statechart::result StateConnected::react(const EventDisconnectRequest&)
{
    post_event(EventDisconnected(L"User requested disconnection."));

    return discard_event();
}

#if 0
static void receiveHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    unsigned char* rcvbuf,
    outermost_context_type& _outermost_context
);
#endif

void StateConnected::sendHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    unsigned char* sendbuf,
    outermost_context_type& _outermost_context
)
{
    // in any case delete the sendbuffer
    delete[] sendbuf;

    // if there was no error, return
    if (!error)
        return;

    // otherwise disconnect
    std::string errmsg(error.message());

    _outermost_context.my_scheduler().queue_event(
        _outermost_context.my_handle(),
        boost::intrusive_ptr<EventDisconnected> (
            new EventDisconnected(
                std::wstring(errmsg.begin(), errmsg.end())
            )
        )
    );
}

// /connect localhost:34443
