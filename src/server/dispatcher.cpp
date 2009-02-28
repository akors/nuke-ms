// dispatcher.cpp

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

#include <algorithm>
#include <functional>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "dispatcher.hpp"

using namespace nms;
using boost::asio::ip::tcp;


DispatchingServer::DispatchingServer()
    : acceptor(io_service, tcp::endpoint(tcp::v4(), listening_port)),
    current_conn_id(0)
{
    startAccept();
}

void DispatchingServer::run() throw()
{
    io_service.run();
}

void DispatchingServer::handleServerEvent(const BasicServerEvent& evt) throw()
{
    // ignore everything that is not in the list
    if (! peers_list.count(evt.connection_id) )
        return;

    switch (evt.event_kind)
    {
        case BasicServerEvent::ID_MSG_RECEIVED:
        {
            const ReceivedMessageEvent& rcvd_msg_evt =
                static_cast<const ReceivedMessageEvent&>(evt);

            std::cout<<"Received a message from "<<rcvd_msg_evt.connection_id<<
                std::endl;

            distributeMessage(rcvd_msg_evt.connection_id, rcvd_msg_evt.parm);

            break;
        }

        case BasicServerEvent::ID_CONNECTION_ERROR:
        {

            const ConnectionErrorEvent& error_evt =
                static_cast<const ConnectionErrorEvent&>(evt);

            std::cout<<"An error with the connection("<<
                error_evt.connection_id<<") occured: "<<
                std::string(error_evt.parm.begin(), error_evt.parm.end())<<
                ". Closing this connection."<<std::endl;

            peers_list[error_evt.connection_id]->shutdownConnection();

            break;
        }

        case BasicServerEvent::ID_CAN_DELETE:
        {
            // delete the peer object if it existed
            peers_list.erase(evt.connection_id);
            break;
        }

        default:
        {
//             bool unknown_server_event = false;
//             assert(unknown_server_event);
            std::cout<<"Unknown server event\n";
            break;
        }
    }
}

void DispatchingServer::startAccept() throw()
{
    // create new socket
    socket_ptr socket(new tcp::socket(io_service));

    // start accept, bind socket to the handler
    acceptor.async_accept(
        *socket,
        boost::bind(
            &DispatchingServer::acceptHandler,
            this,
            boost::asio::placeholders::error,
            socket
        )
    );
}


void DispatchingServer::acceptHandler(
    const boost::system::error_code& e,
    socket_ptr peer_socket
) throw()
{
    if (e)
    {
        std::cout<<"Accepting new clients failed due to an error: "<<
            e.message()<<'\n';

        io_service.stop();
    }
    else
    {
        std::cout<<"New client connected!\n";

        RemotePeer::connection_id_t connection_id = getNextConnectionId();

        // create new peer object
        RemotePeer::ptr_type remote_peer(
            new RemotePeer(
                peer_socket,
                connection_id,
                boost::bind(
                    &DispatchingServer::handleServerEvent,
                    this,
                    _1
                )
            )
        );

        // put peer object into the map
        peers_list[connection_id] = remote_peer;

        startAccept();
    }

}


void DispatchingServer::distributeMessage(
    RemotePeer::connection_id_t originating_id,
    SegmentationLayer::dataptr_type data
)
{
    SegmentationLayer segmlayer_data(*data);

    std::map<RemotePeer::connection_id_t, RemotePeer::ptr_type>::iterator it =
        peers_list.begin();

    for(; it != peers_list.end(); ++it )
    {
        it->second->sendMessage(segmlayer_data);
    }
}

RemotePeer::connection_id_t DispatchingServer::getNextConnectionId()
{
    return ++current_conn_id;
}



