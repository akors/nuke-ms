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

#include <boost/tokenizer.hpp>

#include "protocol.hpp"


void NMSProtocol::connect_to(const std::wstring& id)
{

    if (socket.is_open())
        throw ProtocolError("Allready connected.");   
    
        
    // hopefully the string is made up of the hostname and the service name
    // seperated by a colon
    
    // get ourself a tokenizer
    typedef boost::tokenizer<boost::char_separator<wchar_t>,
                             std::wstring::const_iterator, std::wstring >
        tokenizer;        
        
    // get the part before the colon and the part after the colon
    boost::char_separator<wchar_t> colons(L":");
    tokenizer tokens(id, colons);
    
    tokenizer::iterator tok_iter = tokens.begin();

    // bail out, if there were no tokens
    if ( tok_iter == tokens.end() )
        throw ProtocolError("Invalid remote site identifier.");
        
    // create host from first token
    std::string host(tok_iter->begin(), tok_iter->end());
    
    if ( ++tok_iter == tokens.end() )
        throw ProtocolError("Invalid remote site identifier.");
        
    // create service from second token
    std::string service(tok_iter->begin(), tok_iter->end());
    
    
    try 
    {   
        boost::asio::ip::tcp::resolver resolver(io_service); // get a resolver
        boost::asio::ip::tcp::resolver::query query(host, service); // create a query
        // resolve the query
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query); 
        
        socket.connect(*iterator); // connect to the first found host
    } 
    catch(const boost::system::system_error& e) 
    {
        boost::system::error_code ignored;
        socket.close(ignored);
        throw ProtocolError(e.what());
    }
    
}

void NMSProtocol::send(const std::wstring& msg)
{
    // turn std::wstring into a void* buffer, and put it on the wire
    socket.send(boost::asio::buffer(reinterpret_cast<const void*>(msg.c_str()), 
            msg.length() * sizeof(std::wstring::value_type)));
}

void NMSProtocol::disconnect()
{
    // close the socket, don't care for errors
    try { socket.close(); } catch(...) {}
}


bool NMSProtocol::is_connected() const
{
    return socket.is_open();
}
    
    
    
