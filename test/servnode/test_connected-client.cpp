// test_connected-client.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2011  Alexander Korsunsky
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

#include <iostream>
#include <boost/asio.hpp>

#include "neartypes.hpp"

#include "testutils.hpp"

using namespace nuke_ms;
using namespace boost::asio::ip;
using boost::asio::ip::tcp;

DECLARE_TEST("class ConnectedClient")


void acceptHandler(const boost::system::error_code& accept_error)
{
    TEST_ASSERT(!accept_error);
    std::cout<<(!accept_error ?
        "Someone connected!\n" :
        "Something went wrong with accepting\n")
    ;
}

void connectHandler(const boost::system::error_code& connect_error)
{
    TEST_ASSERT(!connect_error);
    std::cout<<
        (!connect_error ? "Connection succeeded.\n" : "Connection failed.\n");
}

struct SendHandler
{
    std::shared_ptr<byte_traits::byte_sequence> seq;

    void operator() (
        const boost::system::error_code& send_error,
        std::size_t bytes_transferred
    )
    {
        TEST_ASSERT(!send_error && bytes_transferred == seq->size());
        std::cout<<(!send_error ? "Message sent.\n" : "send failed\n");
    }
};

void async_sendMessage(tcp::socket& sock)
{
    NearUserMessage msg{std::move(std::string("Wazzup!!"))};
    auto seq = std::make_shared<byte_traits::byte_sequence>(msg.size());
    msg.fillSerialized(seq->begin());

    std::cout<<"Sending Message... ";
    boost::asio::async_write(sock, boost::asio::buffer(*seq), SendHandler{seq});
}


int main()
{
    boost::asio::io_service io_service;

    tcp::acceptor acceptor_v4{io_service, tcp::endpoint{tcp::v4(), 34443}};
    tcp::socket acc_socket{io_service};

    tcp::socket con_socket{io_service};

    // start accept, bind socket to the handler
    acceptor_v4.async_accept(
        acc_socket,
        &acceptHandler
    );

    // connect to the local socket
    con_socket.async_connect(
        tcp::endpoint{boost::asio::ip::address::from_string("127.0.0.1"),34443},
        &connectHandler
    );

    async_sendMessage(con_socket);


    io_service.run();
    return CONCLUDE_TEST();
}
