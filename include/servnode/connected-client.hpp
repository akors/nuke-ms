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

#include <boost/function.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/asio/io_service.hpp>
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
    boost::asio::io_service& io_service;
    boost::asio::ip::tcp::socket socket;

    // private constructor
    ConnectedClient(
        connection_id_t connection_id,
        boost::asio::ip::tcp::socket&& socket,
        boost::asio::io_service& io_service
    );

    ConnectedClient(ConnectedClient&&) = default;

    // copy construction disallowed
    ConnectedClient(const ConnectedClient&) = delete;

    void async_write(const std::shared_ptr<byte_traits::byte_sequence>& data);

    friend class SendHandler;
    friend class ReceiveHeaderHandler;
    friend class ReceiveBodyHandler;
public:
    struct Signals
    {
        typedef boost::signals2::signal<
            void (
                std::shared_ptr<ConnectedClient>,
                std::shared_ptr<SerializedData>
            )
        > ReceivedMessage;

        typedef boost::signals2::signal<
            void (const std::shared_ptr<ConnectedClient>&)
        > Disconnected;

        boost::signals2::connection
        connectReceivedMessage(const ReceivedMessage::slot_type& slot);

        void disconnectReceivedMessage()
        { receivedMessage.disconnect_all_slots(); }

        template<typename S> void disconnectReceivedMessage(const S& slot_func)
        { receivedMessage.disconnect(slot_func); }

        boost::signals2::connection
        connectDisconnected(const Disconnected::slot_type& slot)
        { return disconnected.connect(slot); }

        void disconnectDisconnected() { disconnected.disconnect_all_slots(); }

        friend class SendHandler;
        friend class ReceiveHeaderHandler;
        friend class ReceiveBodyHandler;

    private:
        ReceivedMessage receivedMessage;
        Disconnected disconnected;

        boost::signals2::connection connectionReceivedMessage;
    } signals;

    connection_id_t connection_id;

    ~ConnectedClient();

    static std::shared_ptr<ConnectedClient> makeInstance(
        connection_id_t connection_id_,
        boost::asio::ip::tcp::socket&& socket_,
        boost::asio::io_service& io_service_,
        boost::function<
            void (
                const std::shared_ptr<ConnectedClient>&,
                const std::shared_ptr<SerializedData>&
            )
        > received_callback,
        boost::function<
            void (const std::shared_ptr<ConnectedClient>&)
        > error_callback
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
};

template <typename InnerLayer>
void ConnectedClient::sendPacket(const SegmentationLayer<InnerLayer>& packet)
{
    // create buffer, fill it with the serialized packet
    auto data = std::make_shared<byte_traits::byte_sequence>(packet.size());
    packet.fillSerialized(data->begin());

    this->async_write(data);
}


} // namespace servnode
} // namespace nuke_ms


#endif // ifndef CONNECTED_CLIENT_HPP_INCLUDED
