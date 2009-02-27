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

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "dispatcher.hpp"

using namespace nms;
using boost::asio::ip::tcp;


DispatchingServer::DispatchingServer()
    : acceptor(io_service, tcp::endpoint(tcp::v4(), listening_port))
{
    startAccept();
}

void DispatchingServer::run() throw()
{
    io_service.run();
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
    socket_ptr socket
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

        const char* msg = "Hello Goodbye!";

        SegmentationLayer::dataptr_type sendbuf(
            new byte_traits::byte_sequence(msg, msg+strlen(msg))
        );

        boost::asio::async_write(
            *socket,
            boost::asio::buffer(*sendbuf),
            boost::bind(
                &RemotePeer::_sendHandler,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred,
                sendbuf
            )
        );

        startAccept();
    }

}
