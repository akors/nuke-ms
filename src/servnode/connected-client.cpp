// connected-client.cpp

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

#include "servnode/connected-client.hpp"

using namespace nuke_ms;
using namespace servnode;

ConnectedClient::ConnectedClient(
    connection_id_t connection_id,
    boost::asio::ip::tcp::socket&& socket,
    boost::asio::io_service& io_service
) : _connection_id(connection_id), _io_service(io_service),
    _socket(std::move(socket))
{ }


boost::signals2::connection ConnectedClient::Signals::connectReceivedMessage(
    const ReceivedMessage::slot_type& slot
)
{
    // disconnect old connection
    _connectionReceivedMessage.disconnect();

    // connect new one
    _connectionReceivedMessage = _receivedMessage.connect(slot);
    return _connectionReceivedMessage;
}

std::shared_ptr<ConnectedClient> ConnectedClient::makeInstance(
    connection_id_t connection_id,
    boost::asio::ip::tcp::socket&& socket,
    boost::asio::io_service& io_service,
    boost::function<void (std::shared_ptr<NearUserMessage>)> received_callback,
    boost::function<void ()> error_callback
)
{
    std::shared_ptr<ConnectedClient> client(new ConnectedClient(
        connection_id,
        std::move(socket),
        io_service
    ));

    client->_signals.connectReceivedMessage(received_callback);
    client->_signals.connectDisconnected(error_callback);

    client->startReceive();

    return client;
}
