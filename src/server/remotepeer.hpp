// remotepeer.hpp

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

#ifndef REMOTEPEER_HPP
#define REMOTEPEER_HPP

#include <boost/asio.hpp>

#include "msglayer.hpp"
#include "servevent.hpp"

namespace nms {

class RemotePeer {

    /** Typedef for pointer to a socket */
    typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

    /** Type that identifies the connection to which this event happened. */
    typedef BasicServerEvent::connection_id_t connection_id_t;

    socket_ptr socket; /**< The socket this Peer is associated with */

    /**< An ID to identify the Peer at the server */
    connection_id_t connection_id;

    /** Callback where events will be reported.*/
    event_callback_t event_callback;


public:
    RemotePeer(
        socket_ptr _socket,
        connection_id_t _connection_id,
        event_callback_t _event_callback
    ) throw();

    void sendHandler(
        const boost::system::error_code& e,
        std::size_t bytes_transferred,
        SegmentationLayer::dataptr_type sendbuf
    ) throw();

    void rcvHeaderHandler(

    );

    void rcvBodyHandler(

    );
};


} // namespace nms

#endif // ifndef REMOTEPEER_HPP