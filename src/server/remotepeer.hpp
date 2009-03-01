// remotepeer.hpp

/*
 *   nuke-ms - Nuclear Messaging System
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

#ifndef REMOTEPEER_HPP
#define REMOTEPEER_HPP

#include <boost/asio.hpp>

#include "msglayer.hpp"
#include "refcounter.hpp"
#include "servevent.hpp"

namespace nuke_ms {

class RemotePeer : protected ReferenceCounter<RemotePeer>
{
    /** Typedef for pointer to a socket */
    typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

public:

    /** Type that identifies the connection to which this event happened. */
    typedef BasicServerEvent::connection_id_t connection_id_t;


    typedef boost::shared_ptr<RemotePeer> ptr_type;


    RemotePeer(
        socket_ptr _peer_socket,
        connection_id_t _connection_id,
        event_callback_t _event_callback
    ) throw();


    void sendMessage(const SegmentationLayer& msg) throw();


    /** Shutdown the connection to the remote peer.
    * This function closes the connected socket.
    * However, deletion of this object is invalid until all handlers have
    * returned.
    * When this has happened, an event with eventtype ID_CAN_DELETE
    * will be sent to the callback specified in the constructor.
    */
    void shutdownConnection() throw();

private:

    socket_ptr peer_socket; /**< The socket this Peer is associated with */

    /**< An ID to identify the Peer at the server */
    const connection_id_t connection_id;

    /** Callback where events will be reported.*/
    event_callback_t event_callback;

    /** A static buffer for the header */
    byte_traits::byte_t header_buffer[SegmentationLayer::header_length];

    /** A variable that will prevent duplicate error messages.
    * Only the first error will be reported. */
    bool error_happened;

    /**
    */
    void startReceive();

    /** Called when all handlers with a this pointer returned.
    * This function should only be called when all handlers that contain a
    * this pointer (also called "member functions") have returned.
    * In this case, the function signals the enclosing entity via the
    * callback that this object can go out of scope.
    */
    void canDelete();

    void postError(const std::wstring& errmsg);

    static void sendHandler(
        const boost::system::error_code& e,
        std::size_t bytes_transferred,
        ReferenceCounter<RemotePeer>::CountedReference peer_reference,
        SegmentationLayer::dataptr_type sendbuf
    ) throw();

    static void rcvHeaderHandler(
        const boost::system::error_code& error,
        std::size_t bytes_transferred,
        ReferenceCounter<RemotePeer>::CountedReference peer_reference
    ) throw();

    static void rcvBodyHandler(
        const boost::system::error_code& error,
        std::size_t bytes_transferred,
        ReferenceCounter<RemotePeer>::CountedReference peer_reference,
        SegmentationLayer::dataptr_type body_data
    ) throw();

    // no copy construction allowed.
    RemotePeer(const RemotePeer&);


};


} // namespace nuke_ms

#endif // ifndef REMOTEPEER_HPP