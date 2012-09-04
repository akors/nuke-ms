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

/** Type for identifiers for currently connected clients */
typedef int connection_id_t;

/** class for currently connected clients.
*
* This class represents clients that are fully connected and ready for communication.
* All negotiation and connection setup must occur with the associated socket *before*
* an instance of this class is created.
*
* Objects of this type cannot be created on the stack and the constructor is inaccessible.
* To create an instance, use the ConnectedClient::makeInstance() static member function.
*
* Signals:
*  - Signals::ReceivedMessage: Packet is received
*  - Signals::Disconnected: Client has disconnected
*
* Slots:
*  - shutdown(): Shutdown connection and disconnect
*  - sendPacket(): Send packet to client
*/
class ConnectedClient
    : public std::enable_shared_from_this<ConnectedClient>
{
    /** Signals sent by ConnectedClient */
    struct Signals
    {
        /** Message received
        * @param connection_id The identifier of the connection the message came from
        * @param data Data received
        */
        typedef std::function<
            void (
                connection_id_t /* connection_id */,
                const std::shared_ptr<SerializedData>& /* data */
            )
        > ReceivedMessage;

        /** Client disconnected
        * @param connection_id The identifier of the client that disconnected
        */
        typedef std::function<
            void (connection_id_t /* connection_id */)
        > Disconnected;

       /** Constructor. Binds callbacks. */
        Signals(const ReceivedMessage& r, const Disconnected& d)
            : receivedMessage{r}, disconnected{d}
        {}

        ReceivedMessage receivedMessage; /**< ReceivedMessage callback */
        Disconnected disconnected; /**< Disconnected callback */
    } signals;

public:
    connection_id_t connection_id; /**< Connection identifier */

    /** Default move constructor. */
    ConnectedClient(ConnectedClient&&) = default;

    /** Destructor.
    *
    * Disconnects the socket.
    */
    ~ConnectedClient();

    /** Create an instance of a ConnectedClient.
    *
    * This factory function creates an instance of ConnectedClient and returns a
    * shared_ptr to it.
    * Pass a socket that is fully connected to a client and has already
    * undergone all necessary session negotiation.
    *
    * You will be notified of the Signals::ReceivedMessage and
    * Signals::Disconnected events via the callbacks you pass in rcvd_callback
    * and rcvd_callback. The connection_id you passed here will passed as
    * argument in every callback invocation.
    *
    * @param connection_id The connection identifier. This identifier can be
    * used by any parent structure and will be passed as argument in the
    * callbacks.
    * @param socket A fully connected and fully set up socket to the remote
    * client.
    * @param rcvd_callback Callback invoked when a new message is received.
    * @param disconnected_callback Callback invoked when the client disconnects
    *
    * @note The callbacks will not be called if the last shared_ptr to this
    * object is destroyed.
    */
    static std::shared_ptr<ConnectedClient> makeInstance(
        connection_id_t connection_id,
        boost::asio::ip::tcp::socket&& socket,
        const Signals::ReceivedMessage& rcvd_callback,
        const Signals::Disconnected& disconnected_callback
    );

    /** Disconnect from client.
    * Shut down the connected socket.
    *
    * @note No packets are sent by this function. If you need to send goodbye
    * spackets, do it before calling it.
    */
    void shutdown();

    /** Send packet to client.
     * Send a packet of a Segmentation layer to the connected client.
     *
     * @tparam InnerLayer type of the payload the SegmentationLayer object is
     * carrying.
     *
     * @param packet Packet to be sent.
     */
    template <typename InnerLayer>
    void sendPacket(const SegmentationLayer<InnerLayer>& packet);

    /** @overload sendPacket(SegmentationLayer<InnerLayer>&& packet) */
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

    /** Socket connected to the remote client */
    boost::asio::ip::tcp::socket socket;

    /** One array as the buffer for all packet headers.
     * This array will be reused by all asynchronous read operations waiting for
     * a new packet.
     */
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

    /** Start receiving messages.
     * This function should be called when all ressources and callbacks are
     * set up and the object is ready to receive data.
    */
    void startReceive();

    /** Invoke asynchronous send operation on socket. */
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
