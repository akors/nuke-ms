// connected-client.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2011, 2012  Alexander Korsunsky
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

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>


using namespace nuke_ms;
using namespace nuke_ms::servnode;
using namespace boost::asio::ip;

namespace nuke_ms { namespace servnode {


template
void ConnectedClient::sendPacket(const SegmentationLayer<SerializedData>&);

template
void ConnectedClient::sendPacket(const SegmentationLayer<NearUserMessage>&);

struct SendHandler
{
    std::weak_ptr<ConnectedClient> parent;
    std::shared_ptr<byte_traits::byte_sequence> buffer;

    void operator() (
        const boost::system::error_code& error,
        std::size_t bytes_transferred
    );
};

struct ReceiveHeaderHandler
{
    std::weak_ptr<ConnectedClient> parent;
    std::shared_ptr<
        std::array<byte_traits::byte_t, SegmentationLayerBase::header_length>
    > buffer;

    ReceiveHeaderHandler(const std::shared_ptr<ConnectedClient>& parent_)
        : parent{parent_}, buffer{parent_->header_buffer}
    {}

    ReceiveHeaderHandler(const ReceiveHeaderHandler& other) = default;

    ReceiveHeaderHandler(ReceiveHeaderHandler&& other)
        : parent{other.parent}, buffer{std::move(other.buffer) }
    {}

    void operator() (
        const boost::system::error_code& error,
        std::size_t bytes_transferred
    );
};

struct ReceiveBodyHandler
{
    std::weak_ptr<ConnectedClient> parent;
    std::shared_ptr<const byte_traits::byte_sequence> buffer;

    void operator() (
        const boost::system::error_code& error,
        std::size_t bytes_transferred
    );
};

}}


ConnectedClient::ConnectedClient(
        connection_id_t connection_id_,
        boost::asio::ip::tcp::socket&& socket_,
        const Signals::ReceivedMessage& rcvd_callback,
        const Signals::Disconnected& disconnected_callback
) : connection_id{connection_id_}, socket{std::move(socket_)},
    header_buffer{std::make_shared<
        std::array<byte_traits::byte_t,SegmentationLayerBase::header_length>
    >()},
    signals{rcvd_callback, disconnected_callback}
{ }

void ConnectedClient::async_write(
    const std::shared_ptr<byte_traits::byte_sequence>& data)
{
    boost::asio::async_write(
        socket,
        boost::asio::buffer(*data),
        SendHandler{shared_from_this(), data}
    );
}



std::shared_ptr<ConnectedClient> ConnectedClient::makeInstance(
    connection_id_t connection_id,
    boost::asio::ip::tcp::socket&& socket,
    const Signals::ReceivedMessage& rcvd_callback,
    const Signals::Disconnected& disconnected_callback
)
{
    std::shared_ptr<ConnectedClient> client{new ConnectedClient{
        connection_id, std::move(socket), rcvd_callback, disconnected_callback
    }};
    client->startReceive();

    return client;
}


ConnectedClient::~ConnectedClient()
{
    // uhm, what exactly should we do here?
}

void ConnectedClient::startReceive()
{
    ReceiveHeaderHandler handler{shared_from_this()};

    async_read(
        socket,
        boost::asio::buffer(*handler.buffer),
        std::move(handler)
    );
}

void ConnectedClient::shutdown()
{
    // initiate socket shutdown
    boost::system::error_code dontcare;
    socket.shutdown(tcp::socket::shutdown_both, dontcare);
}

void SendHandler::operator() (
    const boost::system::error_code& error,
    std::size_t bytes_transferred
)
{
    auto parent = this->parent.lock();
    if (!parent) return; // Mommy is dead? Ok, then nevermind :-(

    // on error, disconnect parent
    if (error || bytes_transferred != buffer->size())
    {
        parent->shutdown();
        parent->signals.disconnected(parent->connection_id);
        return;
    }
}

void ReceiveHeaderHandler::operator() (
    const boost::system::error_code& error,
    std::size_t bytes_transferred
)
{
    auto parent = this->parent.lock();
    if (!parent) return; // Mommy is dead? Ok, then nevermind :-(

    // if we had an error reading, shutdown and send disconnected event
    if (error)
    {
        parent->shutdown();
        parent->signals.disconnected(parent->connection_id);
        return;
    }

    try
    {
        // decode and verify the header of the message
        SegmentationLayerBase::HeaderType header_data
            = SegmentationLayerBase::decodeHeader(buffer->begin());

/// @todo FIXME Magic number, set to something proper or make configurable
constexpr byte_traits::uint2b_t MAX_PACKETSIZE = 0x8FFF;

        if (header_data.packetsize > MAX_PACKETSIZE)
            throw MsgLayerError("Oversized packet.");

        auto body_buf = std::make_shared<byte_traits::byte_sequence>(
            header_data.packetsize - SegmentationLayerBase::header_length
        );

        // start an asynchronous receive for the body
        async_read(
            parent->socket,
            boost::asio::buffer(*body_buf),
            ReceiveBodyHandler{parent, body_buf}
        );
    }
    // on failure, shutdown and send disconnected event
    catch (const MsgLayerError& e)
    {
        parent->shutdown();
        parent->signals.disconnected(parent->connection_id);
    }
}

void ReceiveBodyHandler::operator() (
    const boost::system::error_code& error,
    std::size_t bytes_transferred
)
{
    auto parent = this->parent.lock();
    if (!parent) return; // Mommy is dead? Ok, then nevermind :-(

    // if we had an error reading, shutdown and send disconnected event
    if (error)
    {
        parent->shutdown();
        parent->signals.disconnected(parent->connection_id);
        return;
    }

    // otherwise, construct message and send signal
    parent->signals.receivedMessage(
        parent->connection_id,
        std::make_shared<SerializedData>(buffer, buffer->begin(),buffer->size())
    );

    // restart receive operation
    parent->startReceive();
}
