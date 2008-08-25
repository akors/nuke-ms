// connection.hpp

/*
 *   NMS - Nuclear Messaging System
 *   Copyright (C) 2008  Alexander Korsunsky
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

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include "control/notifications.hpp"

namespace nms
{
namespace protocol
{


using boost::asio::ip::tcp;


/** This is a class that represents a connection to a remote peer.
* @ingroup proto
*
*/
class NMSConnection
{
    // wait for the thread 3 seconds
    enum { threadwait_ms = 3000 };


    /** The working thread */
    boost::thread worker;

    /** The IO service object */
    boost::asio::io_service io_service;

    /** The socket for the connection */
    tcp::socket socket;

    /** The callback for notifications. */
    control::notif_callback_t notification_callback;

    void resolveHandler(const boost::system::error_code& error,
                    tcp::resolver::iterator endpoint_iterator,
                    const std::wstring& id
                    );

    void connectHandler(const boost::system::error_code& error);

    void sendHandler(const boost::system::error_code& error,
                     std::size_t bytes_transferred,
                     unsigned char* sendbuf);

public:

    NMSConnection(const std::wstring& where,
                    const control::notif_callback_t& _notification_callback)
        throw();

    ~NMSConnection()
    {
        disconnect();
    }

    void send(const std::wstring& msg);


    void disconnect() throw();
};

} // namespace protocol
} // namespace nms

#endif // ifdef CONNECTION_HPP
