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
    socket.close(dontcare);

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
            control::SendReport(
                StringwrapLayer(*evt.data->getPayload()).getString(),
                std::wstring(L"Not Connected."))
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

}

boost::statechart::result StateNegotiating::react(const EvtSendMsg& evt)
{
    // we dont send messages to nowhere
    context<ProtocolMachine>()
        .notification_callback(
            control::SendReport(
                StringwrapLayer(*evt.data->getPayload()).getString(),
                std::wstring(L"Not yet Connected."))
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
                        std::wstring(evt.message)
                    )
                );

        return transit<StateWaiting>();
    }
}

boost::statechart::result StateNegotiating::react(const EvtDisconnectRequest&)
{
    return transit<StateWaiting>();
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

        return;
    }

    std::cout<<"Resolving finished. Host: "<<
        endpoint_iterator->endpoint().address().to_string()<<" Port: "<<
        endpoint_iterator->endpoint().port()<<'\n';

    _outermost_context.socket.async_connect(
        *endpoint_iterator,
        boost::bind(
            &StateNegotiating::connectHandler,
            boost::asio::placeholders::error,
            boost::ref(_outermost_context)
        )
    );
}


void StateNegotiating::connectHandler(
    const boost::system::error_code& error,
    outermost_context_type& _outermost_context
)
{
    std::cout<<"connectHandler invoked.\n";

    boost::intrusive_ptr<EvtConnectReport> evt_rprt;

    // if there was an error, create a negative reply
    if (error)
    {
        std::string errmsg(error.message());

        evt_rprt = new EvtConnectReport(
            false,
            std::wstring(errmsg.begin(), errmsg.end())
        );
    }
    else // if there was no error, create a positive reply
    {
        evt_rprt = new EvtConnectReport(
            true,
            std::wstring(L"Connection succeeded.")
        );

        // create a receive buffer
        byte_traits::byte_t* rcvbuf =
            new byte_traits::byte_t[SegmentationLayer::header_length];

        // start an asynchronous read to receive the header of the first packet
        async_read(
            _outermost_context.socket,
            boost::asio::buffer(rcvbuf, SegmentationLayer::header_length),
            boost::bind(
                &StateConnected::receiveSegmentationHeaderHandler,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred,
                boost::ref(_outermost_context),
                rcvbuf
            )
        );
    }

    _outermost_context.my_scheduler().
        queue_event(_outermost_context.my_handle(), evt_rprt);
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
    SegmentationLayer segm_layer(*evt.data);
    SegmentationLayer::dataptr_type data = segm_layer.serialize();

    async_write(
        context<ProtocolMachine>().socket,
        boost::asio::buffer(*data),
        boost::bind(
            &StateConnected::writeHandler,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            boost::ref(outermost_context()),
            data
        )
    );

    return discard_event();
}


boost::statechart::result StateConnected::react(const EvtDisconnected& evt)
{
    context<ProtocolMachine>()
        .notification_callback(control::DisconnectedNotification(evt.msg));

    return transit<StateWaiting>();
}


boost::statechart::result StateConnected::react(const EvtRcvdMessage& evt)
{
     context<ProtocolMachine>()
        .notification_callback(control::ReceivedMsgNotification(evt.msg));

    return discard_event();
}


void StateConnected::writeHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    outermost_context_type& _outermost_context,
    SegmentationLayer::dataptr_type data
)
{
    std::cout<<"writeHandler invoked.\n";

    if (!error)
    {
        _outermost_context.notification_callback(
            control::SendReport(
                StringwrapLayer(*data).getString()
            )
        );
    }
    else
    {
        std::string errmsg(error.message());

        _outermost_context.notification_callback(
            control::SendReport(
                StringwrapLayer(*data).getString(),
                std::wstring(errmsg.begin(), errmsg.end())
            )
        );

        _outermost_context.my_scheduler().queue_event(
            _outermost_context.my_handle(),
            boost::intrusive_ptr<EvtDisconnected> (
                new EvtDisconnected(
                    std::wstring(errmsg.begin(), errmsg.end())
                )
            )
        );

    }
}

void StateConnected::receiveSegmentationHeaderHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    outermost_context_type& _outermost_context,
    byte_traits::byte_t rcvbuf[SegmentationLayer::header_length]
)
{
    std::cout<<"StateConnected::receiveSegmentationHeaderHandler invoked\n";

    // if there was an error,
    // tear down the connection by posting a disconnection event
    if (error || bytes_transferred != SegmentationLayer::header_length)
    {
        std::string errmsg(error.message());

        _outermost_context.my_scheduler().queue_event(
            _outermost_context.my_handle(),
            boost::intrusive_ptr<EvtDisconnected>(
                new EvtDisconnected(std::wstring(errmsg.begin(),errmsg.end()))
            )
        );
    }
    else // if no error occured, try to decode the header
    {
        try {
            // decode and verify the header of the message
            SegmentationLayer::HeaderType header_data
                = SegmentationLayer::decodeHeader(rcvbuf);

/// FIXME Magic number, set to something proper or make configurable
const byte_traits::uint2b_t MAX_PACKETSIZE = 0x8FFF;

            if (header_data.packetsize > MAX_PACKETSIZE)
                throw MsgLayerError("Oversized packet.");


            SegmentationLayer::dataptr_type body_buf(
                new byte_traits::byte_sequence(
                    header_data.packetsize-SegmentationLayer::header_length
                )
            );

            // start an asynchronous receive for the body
            async_read(
                _outermost_context.socket,
                boost::asio::buffer(*body_buf),
                boost::bind(
                    &StateConnected::receiveSegmentationBodyHandler,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    boost::ref(_outermost_context),
                    body_buf
                )
            );
        }
        // on failure, report back to application
        catch (const std::exception& e)
        {
            std::string errmsg(e.what());

            _outermost_context.my_scheduler().queue_event(
                _outermost_context.my_handle(),
                boost::intrusive_ptr<EvtDisconnected>(
                    new EvtDisconnected(
                        std::wstring(errmsg.begin(),errmsg.end())
                    )
                )
            );
        }
        catch(...)
        {
            _outermost_context.my_scheduler().queue_event(
                _outermost_context.my_handle(),
                boost::intrusive_ptr<EvtDisconnected>(
                    new EvtDisconnected(L"Unknown Error")
                )
            );
        }
    }

    delete[] rcvbuf;
}

void StateConnected::receiveSegmentationBodyHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    outermost_context_type& _outermost_context,
    SegmentationLayer::dataptr_type rcvbuf
)
{
    // if there was an error,
    // tear down the connection by posting a disconnection event
    if (error)
    {
        std::string errmsg(error.message());

        _outermost_context.my_scheduler().queue_event(
            _outermost_context.my_handle(),
            boost::intrusive_ptr<EvtDisconnected>(
                new EvtDisconnected(std::wstring(errmsg.begin(),errmsg.end()))
            )
        );
    }
    else // if no error occured, report the received message to the application
    {
        StringwrapLayer strlayer(*rcvbuf);

        _outermost_context.my_scheduler().queue_event(
            _outermost_context.my_handle(),
            boost::intrusive_ptr<EvtRcvdMessage>(
                new EvtRcvdMessage(strlayer.getString())
            )
        );

        // start a new receive for the next message

        // create a receive buffer
        byte_traits::byte_t* rcvbuf =
            new byte_traits::byte_t[SegmentationLayer::header_length];

        // start an asynchronous read to receive the header of the first packet
        async_read(
            _outermost_context.socket,
            boost::asio::buffer(rcvbuf, SegmentationLayer::header_length),
            boost::bind(
                &StateConnected::receiveSegmentationHeaderHandler,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred,
                boost::ref(_outermost_context),
                rcvbuf
            )
        );
    }
}

