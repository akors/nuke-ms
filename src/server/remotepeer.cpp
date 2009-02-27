// remotepeer.cpp

/*
 *   NMS - Nuclear Messaging System
 *   Copyright (C) 2009  Alexander Korsunsky
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

#include "remotepeer.hpp"

#include <boost/bind.hpp>

using namespace nms;


RemotePeer::RemotePeer(
    socket_ptr _peer_socket,
    connection_id_t _connection_id,
    event_callback_t _event_callback
) throw()
    : ReferenceCounter<RemotePeer>(boost::bind(&RemotePeer::canDelete, this)),
    peer_socket(_peer_socket), connection_id(_connection_id),
    event_callback(_event_callback)
{

}

void RemotePeer::startReceive()
{
    // start an asynchrous read
    async_read(
        *peer_socket,
        boost::asio::buffer(header_buffer, SegmentationLayer::header_length),
        boost::bind(
            &RemotePeer::rcvHeaderHandler,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            ReferenceCounter<RemotePeer>::CountedReference(*this)
        )
    );
}

void RemotePeer::sendHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    ReferenceCounter<RemotePeer>::CountedReference peer_reference,
    SegmentationLayer::dataptr_type sendbuf
) throw()
{
    // import reference for convenience
    RemotePeer& remotepeer = peer_reference;

    // nothing to do if everything went fine
    if (!error)
        return;

    // otherwise report error and shutdown connection
    std::string errmsg(error.message());
    remotepeer.disconnection_reason.assign(errmsg.begin(),errmsg.end());

    remotepeer.shutdownConnection();
}

void RemotePeer::rcvHeaderHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    ReferenceCounter<RemotePeer>::CountedReference peer_reference
) throw()
{
    // import reference for convenience
    RemotePeer& remotepeer = peer_reference;

    // on error, set the error string. The rest will be handled by the refernce
    // counter.
    if (error)
    {
        std::string errmsg(error.message());
        remotepeer.disconnection_reason.assign(errmsg.begin(), errmsg.end());
    }
    else
    {
        try {
            // validate header and get packet size
            SegmentationLayer::HeaderType header(
                SegmentationLayer::decodeHeader(remotepeer.header_buffer)
            );

            SegmentationLayer::dataptr_type body_data(
                new byte_traits::byte_sequence(header.packetsize)
            );

            // start a receive for the packet body_data
            async_read(
                *remotepeer.peer_socket,
                boost::asio::buffer(*body_data, header.packetsize),
                boost::bind(
                    &RemotePeer::rcvBodyHandler,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    peer_reference,
                    body_data
                )
            );
        }
        catch(const InvalidHeaderError& e)
        {
            std::string errmsg(error.message());
            remotepeer.disconnection_reason.assign(errmsg.begin(),errmsg.end());
        }

    }
}

void RemotePeer::rcvBodyHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    ReferenceCounter<RemotePeer>::CountedReference peer_reference,
    SegmentationLayer::dataptr_type body_data
) throw()
{
    RemotePeer& remotepeer = peer_reference;

    // on error, set the error string. The rest will be handled by the refernce
    // counter.
    if (error)
    {
        std::string errmsg(error.message());
        remotepeer.disconnection_reason.assign(errmsg.begin(), errmsg.end());
    }
    else
    {
        // if the receive was ok, post the passage back to the enclosing entity
        remotepeer.event_callback(
            ReceivedMessageEvent(remotepeer.connection_id, body_data)
        );

        // renew receive Call
        remotepeer.startReceive();
    }
}

void RemotePeer::canDelete()
{
    if (this->getRefCount() == 0)
        event_callback(
            CanDeleteEvent(connection_id, disconnection_reason)
        );
}



void RemotePeer::_sendHandler(
    const boost::system::error_code& e,
    std::size_t bytes_transferred,
    SegmentationLayer::dataptr_type sendbuf
) throw()
{
    if (e)
        std::cout<<"Sending message failed: "<<e.message()<<'\n';

}

void RemotePeer::sendMessage(const SegmentationLayer& msg) throw()
{
    SegmentationLayer::dataptr_type data = msg.serialize();

    // write the Message onto the line
    boost::asio::async_write(
        *peer_socket,
        boost::asio::buffer(*data),
        boost::bind(
            &RemotePeer::sendHandler,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            ReferenceCounter<RemotePeer>::CountedReference(*this),
            data
        )
    );
}

void RemotePeer::shutdownConnection() throw()
{
    peer_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    peer_socket->close();
}
