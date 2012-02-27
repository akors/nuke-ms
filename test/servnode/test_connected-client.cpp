// test_connected-client.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2012  Alexander Korsunsky
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
#include <boost/thread.hpp>

#include "neartypes.hpp"
#include "servnode/connected-client.hpp"

#include "testutils.hpp"

using namespace nuke_ms;
using namespace boost::asio::ip;
using boost::asio::ip::tcp;

DECLARE_TEST("class ConnectedClient")

// Test against the identity of this data
static std::string data_in_received;

void receiveCallback(
    std::shared_ptr<servnode::ConnectedClient> client,
    std::shared_ptr<SerializedData> data
)
{
    data_in_received.assign(NearUserMessage(*data)._stringwrap._message_string);

    std::cout<<"server: Data received: \""<<data_in_received<<
        "\". Shutting down client.\n";
    client->shutdown();
}

void disconnectCallback(
    std::shared_ptr<servnode::ConnectedClient> client
)
{ std::cout<<"server: Client "<<client->connection_id<<" disconnected.\n"; }



struct MockServer {
    boost::asio::io_service io_service;

    MockServer()
    : acceptor_v4(io_service, tcp::endpoint{tcp::v4(), 34443})
    { }

    void run()
    {
        tcp::socket socket(io_service);

        // start accept, bind socket to the handler
        boost::system::error_code accept_error;
        acceptor_v4.accept(socket, accept_error);

        if (!accept_error)
        {
            std::cout<<"server: Someone connected!\n";
            this->client_container = servnode::ConnectedClient::makeInstance(
                0,
                std::move(socket),
                this->io_service,
                receiveCallback,
                disconnectCallback
            );
        }
        else
            std::cout<<"server: Accept failed with: "<<accept_error.message()<<'\n';

        TEST_ASSERT(!accept_error);

        io_service.run();
    }

// private:
    std::shared_ptr<servnode::ConnectedClient> client_container;
    tcp::acceptor acceptor_v4;
};


void sendMessage(tcp::socket& sock, const std::string& data)
{
    SegmentationLayer<NearUserMessage> msg{NearUserMessage(data)};
    auto seq = std::make_shared<byte_traits::byte_sequence>(msg.size());
    msg.fillSerialized(seq->begin());

    std::cout<<"client: Sending Message...\n";

    boost::system::error_code send_error;
    std::size_t bytes_transferred =
        boost::asio::write(sock, boost::asio::buffer(*seq), send_error);

    std::cout<<(!send_error ? "client: Message sent.\n" : "send failed\n");
    TEST_ASSERT(!send_error && bytes_transferred == seq->size());
}



int main()
{
    // -------- SERVER CODE ---------
    std::cout<<"Initializing server\n";

    MockServer server;

    // Start server with ConnectedClient code
    boost::thread server_thread(boost::bind(&MockServer::run, &server));


    // -------- CLIENT CODE ---------
    std::cout<<"Initializing client\n";

    boost::asio::io_service client_io_service;
    tcp::socket con_socket{client_io_service};

    // connect to the local socket
    boost::system::error_code connect_error;
    con_socket.connect(
        tcp::endpoint{boost::asio::ip::address::from_string("127.0.0.1"),34443},
        connect_error
    );

    TEST_ASSERT(!connect_error);
    if (!connect_error)
        std::cout<<"client: Connection succeeded.\n";
    else
        return 1;

    con_socket.set_option(boost::asio::socket_base::linger(true, 2000));
    sendMessage(con_socket, "Wazzzuuppp???");


    server_thread.join();

    TEST_ASSERT(data_in_received == "Wazzzuuppp???");

    return CONCLUDE_TEST();
}
