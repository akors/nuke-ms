// protocol.hpp

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


/** @defgroup proto Communication Protocl */


#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <stdexcept>

#include <boost/asio.hpp>

#include "control.hpp"


/** Class for errors that can be issued by the Communication Protocol.
*/
class ProtocolError : public std::runtime_error
{
    std::string msg;
public:
    ProtocolError() throw()
        : std::runtime_error("Unknown Communication Protocol Error")
    { }
    
    ProtocolError(const std::string& str) throw()
        : std::runtime_error(str)
    {}
    
    virtual const char* what() throw()
    { return std::runtime_error::what(); }

    virtual ~ProtocolError() throw()
    { }
};

/** Client Communication Protocol.
*/
class NMSProtocol
{
    boost::asio::io_service io_service; /**< IO Service object */
    boost::asio::ip::tcp::socket socket; /**< TCP socket */
     
public:   

    /** Constructor. */
    NMSProtocol()
        : io_service(), socket(io_service)
    { }
    
    
    /** Connect to a remote site.
     * @param id The string representation of the address of the remote site
     */
    void connect_to(const std::wstring& id);
    
    /* Send message to connected remote site.
     * @param msg The message you want to send
     */
    void send(const std::wstring& msg);
    
    /** Disconnect from the remote site.
     * 
     */
    void disconnect();    
    
    bool is_connected(); 
    
};


#endif /*PROTOCOL_HPP_*/
