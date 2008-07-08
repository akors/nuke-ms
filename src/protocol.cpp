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

#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>

#include "protocol.hpp"



void NMSProtocol::connect_to(const std::wstring& id)
{
    // check to see if the socket is allready open
    if (static_cast<tcp::socket&>(resources_safe.get_socket()).is_open())
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

    /** Local Class that contains callback functions. For the asynchronous
    * operation handlers.
    */
    struct AsyncHandler {
        /** Handle a returned Connection request
        * @param error System error code, if there was one
        * @param notification_callback The callback function to be callen when 
        * notifying about events
        */
        static void handleConnect(
                    const boost::system::error_code& error,
                    boost::function1<void, const ProtocolNotification&> 
                        notification_callback)
        {
            if (error)
            {
                std::string errmsg(error.message());
                std::wstring werrmsg(errmsg.begin(), errmsg.end());
                
                notification_callback(
                    ReportNotification
                        <ProtocolNotification::ID_CONNECT_REPORT>
                        (werrmsg)
                    );  
            }  
            else
            {
                notification_callback(
                    ReportNotification
                        <ProtocolNotification::ID_CONNECT_REPORT>
                        ()
                    );  
            }
        }

        /** Handle a returned Resolver request
        * @param error System error code, if there was one
        * @param resources_safe The safe that contains the thread-shared resources
        * @param notification_callback The callback function to be callen when 
        * notifying about events
        *
        * Performs an asynchronous connect. @see handleConnect
        */
        static void handleResolve(const boost::system::error_code& error, 
                        tcp::resolver::iterator endpoint_iterator,
                        ResourcesSafe& resources_safe,
                        boost::function1<void, const ProtocolNotification&> 
                            notification_callback
                        )
        {
            if (error)
            {
                std::string errmsg(error.message());
                std::wstring werrmsg(errmsg.begin(), errmsg.end());
            
            
                notification_callback(
                    ReportNotification
                        <ProtocolNotification::ID_CONNECT_REPORT>
                        (werrmsg)
                    );  
            }
            else
            {                        
                // get ourself exclusive access to the socket
                ResourcesSafe::GuardedSocket socket = 
                    resources_safe.get_socket();
                    
                static_cast<tcp::socket&>(resources_safe.get_socket())
                    .async_connect(
                            *endpoint_iterator,
                            boost::bind(&AsyncHandler::handleConnect,
                                    boost::asio::placeholders::error, 
                                    notification_callback)
                            );
            }                   
        }
    };

    try
    {
        
        tcp::resolver resolver(io_service); // get a resolver    
        tcp::resolver::query query(host, service); // create a query
        
        // resolve the query, connect to the remote site        
        resolver.async_resolve(query, 
                                boost::bind(&AsyncHandler::handleResolve, 
                                            _1, _2, boost::ref(resources_safe),
                                            notification_callback
                                            ));
        
        // create a working thread
        worker.reset(
            new Worker(resources_safe, io_service, notification_callback));
    }
    catch(const boost::system::system_error& e)
    {
        worker.reset(); // delete worker
        throw ProtocolError(e.what());
    }

}

void NMSProtocol::send(const std::wstring& msg)
{
    ResourcesSafe::GuardedSocket socket = resources_safe.get_socket();

    // take the socket from the safe, 
    // turn std::wstring into a void* buffer, and put it on the wire
    static_cast<tcp::socket&>(socket)
        .send(boost::asio::buffer(reinterpret_cast<const void*>(msg.c_str()),
                msg.length() * sizeof(std::wstring::value_type)));
}

void NMSProtocol::disconnect()
{
    // close the socket, don't care for errors
    try { 
        worker.reset();
    
        static_cast<tcp::socket&>(resources_safe.get_socket())
            .close(); 
    } catch(...) {}
}

bool NMSProtocol::is_connected()
{
try {
    return 
    static_cast<tcp::socket&>(resources_safe.get_socket())
        .is_open();
}
catch(const std::exception& e)
{
    std::cerr<<"Caught exception: "<<e.what()<<'\n';
    return false;
}
    
}



