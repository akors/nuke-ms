// statemachine.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2009, 2010  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "clientnode/statemachine.hpp"

using namespace nuke_ms;
using namespace nuke_ms::clientnode;
using namespace boost::asio::ip;


ClientnodeMachine::ClientnodeMachine(ClientNodeSignals&  _signals,
	LoggingStreams logstreams_, boost::mutex& _machine_mutex
)
    : signals(_signals), socket(io_service),
        logstreams(logstreams_), machine_mutex(_machine_mutex)
{}

ClientnodeMachine::~ClientnodeMachine()
{
    stopIOOperations();
}

void ClientnodeMachine::startIOOperations()
{
    // start a new thread that processes all asynchronous operations
    io_thread = boost::thread(
        boost::bind(
            &boost::asio::io_service::run,
            &outermost_context().io_service
        )
    );
}

void ClientnodeMachine::stopIOOperations()
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
    outermost_context().logstreams.infostream<<"Entering StateWaiting"<<
		std::endl;

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
    SendReport::ptr_t rprt(new SendReport);
    rprt->send_state = false;
    rprt->reason = SendReport::SR_SERVER_NOT_CONNECTED;
    rprt->reason_str = "Not Connected.";

    context<ClientnodeMachine>().signals.sendReport(rprt);

    return discard_event();
}



StateNegotiating::StateNegotiating(my_context ctx)
    : my_base(ctx)
{
    outermost_context().logstreams.infostream<<"Entering StateNegotiating"<<
		std::endl;

    try {
        outermost_context().startIOOperations();
    }
    catch(const std::exception& e)
    {
        byte_traits::native_string errmsg(e.what());
        errmsg = "Internal error:" + errmsg;

        post_event(EvtConnectReport(false, errmsg));
    }
    catch(...)
    {
        post_event(EvtConnectReport(false,"Unknown internal Error"));
    }

}

boost::statechart::result StateNegotiating::react(const EvtSendMsg& evt)
{
    SendReport::ptr_t rprt(new SendReport);
    rprt->send_state = false;
    rprt->reason = SendReport::SR_SERVER_NOT_CONNECTED;
    rprt->reason_str = "Not yet Connected.";

    context<ClientnodeMachine>().signals.sendReport(rprt);

    return discard_event();
}

boost::statechart::result StateNegotiating::react(const EvtConnectReport& evt)
{
    ConnectionStatusReport::ptr_t
        rprt(new ConnectionStatusReport);

    // change state according to the outcome of a connection attempt
    if ( evt.success )
    {
        rprt->newstate = ConnectionStatusReport::CNST_CONNECTED;
        rprt->statechange_reason = ConnectionStatusReport::STCHR_USER_REQUESTED;
        rprt->msg = evt.message;
        context<ClientnodeMachine>().signals.connectStatReport(rprt);

        return transit<StateConnected>();
    }
    else
    {
        rprt->newstate = ConnectionStatusReport::CNST_DISCONNECTED;
        rprt->statechange_reason = ConnectionStatusReport::STCHR_CONNECT_FAILED;
        rprt->msg = evt.message;
        context<ClientnodeMachine>().signals.connectStatReport(rprt);

        return transit<StateWaiting>();
    }
}

boost::statechart::result StateNegotiating::react(const EvtDisconnectRequest&)
{
    ConnectionStatusReport::ptr_t rprt(new ConnectionStatusReport);

    rprt->newstate = ConnectionStatusReport::CNST_DISCONNECTED;
    rprt->statechange_reason = ConnectionStatusReport::STCHR_USER_REQUESTED;
    context<ClientnodeMachine>().signals.connectStatReport(rprt);

    return transit<StateWaiting>();
}

boost::statechart::result StateNegotiating::react(const EvtConnectRequest&)
{
    ConnectionStatusReport::ptr_t rprt(new ConnectionStatusReport);

    rprt->newstate = ConnectionStatusReport::CNST_CONNECTING;
    rprt->statechange_reason = ConnectionStatusReport::STCHR_BUSY;
    rprt->msg = "Currently trying to connect";
    context<ClientnodeMachine>().signals.connectStatReport(rprt);

    return discard_event();
}



void StateNegotiating::resolveHandler(
    const boost::system::error_code& error,
    tcp::resolver::iterator endpoint_iterator,
    outermost_context_type& _outermost_context,
    boost::shared_ptr<tcp::resolver> /* resolver */,
    boost::shared_ptr<tcp::resolver::query> /* query */
)
{
    _outermost_context.logstreams.infostream<<
        "resolveHandler invoked."<<std::endl;


    // if there was an error, report it
    if (endpoint_iterator == tcp::resolver::iterator() )
    {
        byte_traits::native_string errmsg;

        if (error)
		{
			// if the operation was aborted, the state machine might not be alive,
			// so we STFU and return
			if (error == boost::asio::error::operation_aborted)
				return;

            errmsg = error.message();
		}
        else
            errmsg = "No hosts found.";

        boost::mutex::scoped_lock(_outermost_context.machine_mutex);
        _outermost_context.process_event(EvtConnectReport(false, errmsg));

        return;
    }

    _outermost_context.logstreams.infostream<<"Resolving finished. Host: "<<
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
    _outermost_context.logstreams.infostream<<"connectHandler invoked."<<std::endl;

    // if there was an error, create a negative reply
    if (error)
    {
		// if the operation was aborted, the state machine might not be alive,
		// so we STFU and return
		if (error == boost::asio::error::operation_aborted)
			return;

        byte_traits::native_string errmsg(error.message());

        boost::mutex::scoped_lock(_outermost_context.machine_mutex);
        _outermost_context.process_event(
            EvtConnectReport(false, errmsg)
        );
    }
    else // if there was no error, create a positive reply
    {
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

        boost::mutex::scoped_lock(_outermost_context.machine_mutex);
        _outermost_context.process_event(
            EvtConnectReport(true,"Connection succeeded.")
        );
    }
}





StateConnected::StateConnected(my_context ctx)
    : my_base(ctx)
{
    outermost_context().logstreams.infostream<<"Entering StateConnected"<<
		std::endl;
}


boost::statechart::result StateConnected::react(const EvtDisconnectRequest&)
{
    ConnectionStatusReport::ptr_t rprt(new ConnectionStatusReport);

    rprt->newstate = ConnectionStatusReport::CNST_DISCONNECTED;
    rprt->statechange_reason = ConnectionStatusReport::STCHR_USER_REQUESTED;
    context<ClientnodeMachine>().signals.connectStatReport(rprt);

    return transit<StateWaiting>();
}


boost::statechart::result StateConnected::react(const EvtSendMsg& evt)
{
    // create segmentation layer from the data to be sent
    SegmentationLayer segm_layer(evt.data);

    // create buffer, fill it with the serialized message
    SegmentationLayer::dataptr_t data(
        new byte_traits::byte_sequence(segm_layer.size())
    );

    segm_layer.fillSerialized(data->begin());

    async_write(
        context<ClientnodeMachine>().socket,
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
    ConnectionStatusReport::ptr_t rprt(new ConnectionStatusReport);

    rprt->newstate = ConnectionStatusReport::CNST_DISCONNECTED;
    rprt->statechange_reason = ConnectionStatusReport::STCHR_SOCKET_CLOSED;
    rprt->msg = evt.msg;
    context<ClientnodeMachine>().signals.connectStatReport(rprt);

    return transit<StateWaiting>();
}


boost::statechart::result StateConnected::react(const EvtRcvdMessage& evt)
{
    // whatever it is, we need a SerializedData object from it
    SerializedData data(evt.data.getUpperLayer()->getSerializedData());


    try {
        // check out the layer identifier if it's a string, dispatch it.
        // If not, discard
        if (*data.getDataIterator() ==
            static_cast<byte_traits::byte_t>(NearUserMessage::LAYER_ID))
        {
            NearUserMessage::ptr_t usermsg(new NearUserMessage(data));
            context<ClientnodeMachine>().signals.rcvMessage(usermsg);
        }
        else
		{
            outermost_context().logstreams.warnstream<<
				"Received packet with unknown layer identifier! Discarding."<<
				std::endl;
		}
    }
    catch(const MsgLayerError& e)
    {
        outermost_context().logstreams.errorstream<<
			"Reiceived packet but failed to create Message object: "<<e.what()<<
			std::endl;
    }

    return discard_event();
}


boost::statechart::result StateConnected::react(const EvtConnectRequest&)
{
    ConnectionStatusReport::ptr_t rprt(new ConnectionStatusReport);

    rprt->newstate = ConnectionStatusReport::CNST_CONNECTED;
    rprt->statechange_reason = ConnectionStatusReport::STCHR_BUSY;
    rprt->msg = "Allready connected";
    context<ClientnodeMachine>().signals.connectStatReport(rprt);

    return discard_event();
}



void StateConnected::writeHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    outermost_context_type& _outermost_context,
    SegmentationLayer::dataptr_t data
)
{
    _outermost_context.logstreams.infostream<<"Sending message finished"<<std::endl;


    if (!error)
    {

        SendReport::ptr_t rprt(new SendReport);
        rprt->send_state = true;
        rprt->reason = SendReport::SR_SEND_OK;

        _outermost_context.signals.sendReport(rprt);
    }
    else
    {
		// if the operation was aborted, the state machine might not be alive,
		// so we STFU and return
		if (error == boost::asio::error::operation_aborted)
			return;


        byte_traits::native_string errmsg(error.message());

        SendReport::ptr_t rprt(new SendReport);
        rprt->send_state = false;
        rprt->reason = SendReport::SR_CONNECTION_ERROR;
        rprt->reason_str = errmsg;

        _outermost_context.signals.sendReport(rprt);

        boost::mutex::scoped_lock(_outermost_context.machine_mutex);
        _outermost_context.process_event(EvtDisconnected(errmsg));

    }
}

void StateConnected::receiveSegmentationHeaderHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    outermost_context_type& _outermost_context,
    byte_traits::byte_t rcvbuf[SegmentationLayer::header_length]
)
{
    _outermost_context.logstreams.infostream<<"Reveived message"<<std::endl;

    // if there was an error,
    // tear down the connection by posting a disconnection event
    if (error || bytes_transferred != SegmentationLayer::header_length)
    {
		// if the operation was aborted, the state machine might not be alive,
		// so we STFU and return
		if (error == boost::asio::error::operation_aborted)
			return;

        byte_traits::native_string errmsg(error.message());

        boost::mutex::scoped_lock(_outermost_context.machine_mutex);
        _outermost_context.process_event(EvtDisconnected(errmsg));
    }
    else // if no error occured, try to decode the header
    {
        try {
            // decode and verify the header of the message
            SegmentationLayer::HeaderType header_data
                = SegmentationLayer::decodeHeader(rcvbuf);

/// @todo Magic number, set to something proper or make configurable
const byte_traits::uint2b_t MAX_PACKETSIZE = 0x8FFF;

            if (header_data.packetsize > MAX_PACKETSIZE)
                throw MsgLayerError("Oversized packet.");


            SegmentationLayer::dataptr_t body_buf(
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
            boost::mutex::scoped_lock(_outermost_context.machine_mutex);
            _outermost_context.process_event(EvtDisconnected(e.what()));
        }
        catch(...)
        {
            boost::mutex::scoped_lock(_outermost_context.machine_mutex);
            _outermost_context.process_event(EvtDisconnected("Unknown Error"));
        }
    }

    delete[] rcvbuf;
}

void StateConnected::receiveSegmentationBodyHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    outermost_context_type& _outermost_context,
    SegmentationLayer::dataptr_t rcvbuf
)
{
    // if there was an error,
    // tear down the connection by posting a disconnection event
    if (error)
    {
		// if the operation was aborted, the state machine might not be alive,
		// so we STFU and return
		if (error == boost::asio::error::operation_aborted)
			return;

        boost::mutex::scoped_lock(_outermost_context.machine_mutex);
        _outermost_context.process_event(EvtDisconnected(error.message()));
    }
    else // if no error occured, report the received message to the application
    {
        SegmentationLayer segmlayer(rcvbuf);

        {
            boost::mutex::scoped_lock(_outermost_context.machine_mutex);
            _outermost_context.process_event(EvtRcvdMessage(segmlayer));
        }
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

