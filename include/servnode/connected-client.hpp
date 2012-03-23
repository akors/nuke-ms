// connected-client.hpp

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

#ifndef CONNECTED_CLIENT_HPP_INCLUDED
#define CONNECTED_CLIENT_HPP_INCLUDED

#include <memory>
#include <array>
#include <functional>

#include <boost/asio/ip/tcp.hpp>

#include "neartypes.hpp"

namespace nuke_ms
{
namespace servnode
{

typedef int connection_id_t;


class ConnectedClient
    : public std::enable_shared_from_this<ConnectedClient>
{
    struct Signals
    {
        typedef std::function<
            void (connection_id_t, const std::shared_ptr<SerializedData>&)
        > ReceivedMessage;

        typedef std::function<
            void (connection_id_t)
        > Disconnected;

        Signals(const ReceivedMessage& r, const Disconnected& d)
            : receivedMessage{r}, disconnected{d}
        {}

        ReceivedMessage receivedMessage;
        Disconnected disconnected;
    } signals;

public:
    connection_id_t connection_id;

    ConnectedClient(ConnectedClient&&) = default;

    ~ConnectedClient();

    static std::shared_ptr<ConnectedClient> makeInstance(
        connection_id_t connection_id,
        boost::asio::ip::tcp::socket&& socket,
        const Signals::ReceivedMessage& rcvd_callback,
        const Signals::Disconnected& disconnected_callback
    );

    void startReceive();

    void shutdown();

    template <typename InnerLayer>
    void sendPacket(const SegmentationLayer<InnerLayer>& packet);

    template <typename InnerLayer>
    void sendPacket(SegmentationLayer<InnerLayer>&& packet)
    {
        // create own copy and redirect
        SegmentationLayer<InnerLayer> data{std::move(packet)};
        sendPacket(data);
    }

private:
    friend class SendHandler;
    friend class ReceiveHeaderHandler;
    friend class ReceiveBodyHandler;

    boost::asio::ip::tcp::socket socket;

    std::shared_ptr<
        std::array<byte_traits::byte_t, SegmentationLayerBase::header_length>
    > header_buffer;

    // private constructor
    ConnectedClient(
        connection_id_t connection_id,
        boost::asio::ip::tcp::socket&& socket,
        const Signals::ReceivedMessage& rcvd_callback,
        const Signals::Disconnected& disconnected_callback
    );

    // copy construction disallowed
    ConnectedClient(const ConnectedClient&) = delete;

    void async_write(const std::shared_ptr<byte_traits::byte_sequence>& data);
};

template <typename InnerLayer>
void ConnectedClient::sendPacket(const SegmentationLayer<InnerLayer>& packet)
{
    // create buffer, fill it with the serialized packet
    auto data = std::make_shared<byte_traits::byte_sequence>(packet.size());
    packet.fillSerialized(data->begin());

    this->async_write(data);
}

extern template
void ConnectedClient::sendPacket(const SegmentationLayer<SerializedData>&);

extern template
void ConnectedClient::sendPacket(const SegmentationLayer<NearUserMessage>&);

} // namespace servnode
} // namespace nuke_ms


#endif // ifndef CONNECTED_CLIENT_HPP_INCLUDED
