// protocol.cpp

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

#include "protocol.hpp"


void NMSProtocol::connect_to(const std::wstring& id)
{

    if (socket.is_open())
        throw NMSProtocolError("Allready connected.");   
    
        
    // hopefully the string is made up of the hostname and the service name
    // seperated by a colon
    std::wstring::const_iterator it = id.begin();
    
    while ( it<id.end() && *++it != ':');
        
    std::string host(id.begin(), it);
    std::string service(++it, id.end());

    
    try {
    
    ba_tcp::resolver resolver(io_service);
    ba_tcp::resolver::query query(host, service);
    ba_tcp::resolver::iterator iterator = resolver.resolve(query);
    
    socket.connect(*iterator);
    

                                                  
    } catch(const boost::system::system_error& e) {
        throw NMSProtocolError("Connecting to " + host + " failed:" + e.what());
    }
    
}

void NMSProtocol::send(const std::wstring& msg)
{
    socket.send(boost::asio::buffer(reinterpret_cast<const void*>(msg.c_str()), 
            msg.length() * sizeof(std::wstring::value_type)));
}

void NMSProtocol::disconnect()
{
    socket.close();
}


bool NMSProtocol::is_connected()
{
    return socket.is_open();
}
    
    
    
