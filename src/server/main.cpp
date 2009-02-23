// main.cpp

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

#include <iostream>

#include "msglayer.hpp"
#include "dispatcher.hpp"

using boost::asio::ip::tcp;

#if 0
class DispatchingServer
{

    typedef boost::shared_ptr<tcp::socket> sockptr;

    static char const message[];
    static const int port = 34443;

    boost::asio::io_service& io_service;
    tcp::acceptor acceptor;

    std::list<sockptr> socklist;

public:
    DispatchingServer(boost::asio::io_service& _io_service)
        : io_service(_io_service),
        acceptor(io_service, tcp::endpoint(tcp::v4(), 34443))
    {
        std::cout<<"  * Starting server\n";

        startAccept();
    }

    void startAccept();

    void acceptHandler(
        const boost::system::error_code& error,
        sockptr newsocket
    );

    void handleNewConnection(sockptr newsocket);

    void receiveHandler(
        const boost::system::error_code& error,
        std::size_t bytes_transferred,
        char* rcvbuf,
        std::list<sockptr>::iterator socket_iterator
    );
};


char const DispatchingServer::message[] = "Hello Goodbye!\n";

void DispatchingServer::startAccept()
{
    sockptr newsocket(new tcp::socket(io_service));

    acceptor.async_accept(
        *newsocket,
        boost::bind(
            &DispatchingServer::acceptHandler,
            this,
            boost::asio::placeholders::error,
            newsocket
        )
    );
}

void DispatchingServer::acceptHandler(
    const boost::system::error_code& error,
    sockptr newsocket
)
{
    if (error)
    {
        std::cout<<"Error accepting connection: "<<error.message()<<'\n';
        return;
    }

    std::cout<<"  > Someone connected\n";

    handleNewConnection(newsocket);
    startAccept();
}


void DispatchingServer::handleNewConnection(sockptr newsocket)
{
    // put connection in socket list
    socklist.push_back(newsocket);

    char *strbuf = new char[10];

    // put the socket onto a receive
    async_read(
        *newsocket,
        boost::asio::buffer(strbuf, 10),
        boost::bind(
            &DispatchingServer::receiveHandler,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            strbuf,
            socklist.end()
        )
    );
}

void DispatchingServer::receiveHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    char* rcvbuf,
    std::list<sockptr>::iterator socket_iterator
)
{
    if ( error )
    {
        std::cout<<"Error receiving the message: "<<error.message()<<'\n';
        socklist.erase(socket_iterator);
    }
    else
    {
        std::cout<<"  > received Message: "<<rcvbuf<<'\n';

        char *strbuf = new char[10];

        // put the socket onto a receive
        async_read(
            **socket_iterator,
            boost::asio::buffer(strbuf, 10),
            boost::bind(
                &DispatchingServer::receiveHandler,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred,
                strbuf,
                socklist.end()
            )
        );
    }

    delete rcvbuf;
}
#endif

int main()
{
    nms::DispatchingServer server;

    server.run();

    std::cout<<"The server is terminating.\n";

    return 0;
}