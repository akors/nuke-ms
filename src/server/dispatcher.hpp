// dispatcher.hpp

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


#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <map>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "remotepeer.hpp"

namespace nuke_ms
{

/** Main server class.
*
* This class represents the main class of the server.
* To use it, create an instance and then call the run() member function.
*
*/
class DispatchingServer
{

public:

    DispatchingServer();

    /** Start the server.
    * This function makes the server begin his work. It will block until the
    * server has finished or an error occured.
    * No exception will be thrown, however output may occur.
    */
    void run() throw();

    void handleServerEvent(const BasicServerEvent& evt) throw();

private:
    typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
    typedef std::map<RemotePeer::connection_id_t, RemotePeer::ptr_type>
        peers_list_type;

    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;

    /** A list with connected peers. */
   peers_list_type peers_list;

    static const unsigned short listening_port = 34443;

    RemotePeer::connection_id_t current_conn_id;

    /** Dispatch an asynchronous accept request.
    * The request will be processed when the run() member function is run.
    */
    void startAccept() throw();

    /**
    * Callback function for completed accept requests.
    */
    void acceptHandler(
        const boost::system::error_code& e,
        socket_ptr peer_socket
    ) throw();

    void distributeMessage(
        RemotePeer::connection_id_t originating_id,
        SegmentationLayer::ptr_type data
    );

    RemotePeer::connection_id_t getNextConnectionId();

};


} //namespace nuke_ms




#endif // ifndef DISPATCHER_HPP
