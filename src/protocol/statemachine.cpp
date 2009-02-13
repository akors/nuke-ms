// statemachine.cpp

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

#include "protocol/statemachine.hpp"

using namespace nms::protocol;
using namespace boost::asio::ip;

ProtocolMachine::ProtocolMachine(my_context ctx,
                    nms::control::notif_callback_t _notification_callback)
    : my_base(ctx),
    notification_callback(_notification_callback)
    ,socket(io_service)
{}

ProtocolMachine::~ProtocolMachine()
{
    stopIOOperations();
}

void ProtocolMachine::startIOOperations()
    throw(std::exception)
{
    // start a new thread that processes all asynchronous operations
    io_thread = boost::thread(
        boost::bind(
            &boost::asio::io_service::run,
            &outermost_context().io_service
        )
    );
}

void ProtocolMachine::stopIOOperations() throw()
{
    boost::system::error_code dontcare;

    // cancel all operations and close the socket
    // socket.close(dontcare);

    // stop the service object if it's running
    io_service.stop();

    // stop the thread
    catchThread(io_thread, thread_timeout);

    // reset the io_service object so it can continue its work afterwards
    io_service.reset();
}


StateWaiting::StateWaiting(my_context ctx)
    : my_base(ctx)
{
    std::cout<<"Entering StateWaiting\n";

    // when we are waiting, we don't need the io_service object and the thread
    outermost_context().stopIOOperations();
}


void StateNegotiating::tiktakHandler(
    const boost::system::error_code& error,
    boost::shared_ptr<boost::asio::deadline_timer> timer,
    outermost_context_type& _outermost_context
)
{
    std::cout<<"Timer went off: "<<error.message()<<'\n';
}

void StateNegotiating::resolveHandler(
    const boost::system::error_code& error,
    tcp::resolver::iterator endpoint_iterator,
    outermost_context_type& _outermost_context,
    boost::shared_ptr<tcp::resolver> /* resolver */,
    boost::shared_ptr<tcp::resolver::query> /* query */
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
            boost::intrusive_ptr<EvtConnectReport> (
                new EvtConnectReport(
                    false,
                    std::wstring(errmsg.begin(),errmsg.end())
                )
            )
        );

        std::cout<<"Connecting failed: "<<errmsg<<'\n';
        return;
    }

    std::cout<<"Resolving finished. Host: "<<
        endpoint_iterator->endpoint().address().to_string()<<" Port: "<<
        endpoint_iterator->endpoint().port()<<'\n';
}


boost::statechart::result StateWaiting::react(const EvtConnectRequest& evt)
{
     //get a resolver
    boost::shared_ptr<tcp::resolver> resolver(
        new tcp::resolver(outermost_context().io_service)
    );

    // create a query
    boost::shared_ptr<tcp::resolver::query> query(
        new tcp::resolver::query(evt.host, evt.service)
    );


    // dispatch an asynchronous resolve request
    resolver->async_resolve(
        *query,
        boost::bind(
            &StateNegotiating::resolveHandler,
            boost::asio::placeholders::error,
            boost::asio::placeholders::iterator,
            boost::ref(outermost_context()),
            resolver, query
        )
    );

#if 0
    boost::shared_ptr<boost::asio::deadline_timer> timer(
        new boost::asio::deadline_timer(
            outermost_context().io_service, boost::posix_time::seconds(5)
        )
    );

    timer->async_wait(
        boost::bind(
            StateNegotiating::tiktakHandler,
            boost::asio::placeholders::error,
            timer,
            boost::ref(outermost_context())
        )
    );
#endif

    return transit< StateNegotiating >();
}

boost::statechart::result StateWaiting::react(const EvtSendMsg& evt)
{
    context<ProtocolMachine>()
        .notification_callback(
            control::SendReport(evt.msg, std::wstring(L"Not Connected."))
            );

    return discard_event();
}



StateNegotiating::StateNegotiating(my_context ctx)
    : my_base(ctx)
{
    std::cout<<"Entering StateNegotiating\n";

    try {
        outermost_context().startIOOperations();
    }
    catch(const std::exception& e)
    {
        std::string errmsg(e.what());
        errmsg = "Internal error:" + errmsg;

        post_event(
            EvtConnectReport(
                false,
                std::wstring(errmsg.begin(), errmsg.end())
            )
        );
    }
    catch(...)
    {
        post_event(
            EvtConnectReport(
                false,
                std::wstring(L"Unknown internal Error")
            )
        );
    }

    // send positive connection report for debugging purposes
    post_event(
        EvtConnectReport(true, std::wstring(L"Connection attempt succeeded."))
    );
}

boost::statechart::result StateNegotiating::react(const EvtSendMsg& evt)
{
    // we dont send messages to nowhere
    context<ProtocolMachine>()
        .notification_callback(
            control::SendReport(evt.msg, std::wstring(L"Not yet Connected."))
            );

    return discard_event();
}

boost::statechart::result StateNegotiating::react(const EvtConnectReport& evt)
{
    // change state according to the outcome of a connection attempt
    if ( evt.success )
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
                    <control::ProtocolNotification::ID_CONNECT_REPORT>(
                        std::wstring(L"Connection Failed: " + evt.message)
                    )
                );

        return transit<StateWaiting>();
    }
}

boost::statechart::result StateNegotiating::react(const EvtDisconnectRequest&)
{
    return transit<StateWaiting>();
}

StateConnected::StateConnected(my_context ctx)
    : my_base(ctx)
{
    std::cout<<"Entering StateConnected\n";
}


boost::statechart::result StateConnected::react(const EvtDisconnectRequest&)
{
    return transit<StateWaiting>();
}


boost::statechart::result StateConnected::react(const EvtSendMsg& evt)
{
#if 1
    std::wcout<<L" << "<<evt.msg<<L'\n';
#endif

    return discard_event();
}
