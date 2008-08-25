// connection.cpp

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

#include "protocol/connection.hpp"
#include "protocol/errors.hpp"



using namespace nms;
using namespace protocol;



NMSConnection::NMSConnection(const std::wstring& where,
                             const control::notif_callback_t&
                                _notification_callback)
    throw()
    : notification_callback(_notification_callback),
      socket(io_service)
{


    try {
        // get ourself a tokenizer
        typedef boost::tokenizer<boost::char_separator<wchar_t>,
                                std::wstring::const_iterator, std::wstring>
            tokenizer;

        // get the part before the colon and the part after the colon
        boost::char_separator<wchar_t> colons(L":");
        tokenizer tokens(where, colons);

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

        // bail out if there is another colon
        if ( ++tok_iter != tokens.end() )
            throw ProtocolError("Invalid remote site identifier.");





        tcp::resolver resolver(io_service); // get a resolver
        tcp::resolver::query query(host, service); // create a query

        // dispatch an asynchronous resolve request
        resolver.async_resolve(
            query,
            boost::bind(
                    &NMSConnection::resolveHandler, this,
                    _1 , _2,
                    where
                )
            );

        // start a new thread that processes all asynchronous operations
        worker = boost::thread(
                    boost::bind(&boost::asio::io_service::run, &io_service)
                    );

    }
    catch (const std::exception& e)
    {
        std::string errmsg(e.what());

        notification_callback(
            control::ReportNotification<
                    control::ProtocolNotification::ID_CONNECT_REPORT
                    >
                (std::wstring(errmsg.begin(), errmsg.end()))
            );
    }

}


void NMSConnection::resolveHandler(const boost::system::error_code& error,
                        tcp::resolver::iterator endpoint_iterator,
                        const std::wstring& id
                       )
{


    // if there was an error, report it
    if (error)
    {
        std::string errmsg(error.message());

        notification_callback(
            control::ReportNotification<
                    control::ProtocolNotification::ID_CONNECT_REPORT
                    >
                (std::wstring(errmsg.begin(), errmsg.end()))
            );
    }


    // otherwise try to connect


    // dispatch an asynchronous connect request
    socket.async_connect(
        *endpoint_iterator,
        boost::bind(&NMSConnection::connectHandler, this, _1)
        );

}




void NMSConnection::connectHandler(const boost::system::error_code& error)
{
    if (error)
    {
        std::string errmsg(error.message());

        // report failure
        notification_callback(
            control::ReportNotification<
                    control::ProtocolNotification::ID_CONNECT_REPORT
                    >
                (std::wstring(errmsg.begin(), errmsg.end()))
            );
    }

    notification_callback(
    control::ReportNotification<
            control::ProtocolNotification::ID_CONNECT_REPORT
            >
        ()
    );
}


void NMSConnection::sendHandler(const boost::system::error_code& /* error */,
    std::size_t /* bytes_transferred */,
    unsigned char* sendbuf
)
{
    // in any case delete the sendbuffer
    delete[] sendbuf;

    // if there was an error, close the socket
    disconnect();
}

void NMSConnection::send(const std::wstring& msg)
{
    std::size_t sendbuf_size = msg.size()*sizeof(std::wstring::value_type);

    unsigned char* sendbuf = new unsigned char[sendbuf_size];

    std::copy(msg.begin(), msg.end(),
        reinterpret_cast<std::wstring::value_type*>(sendbuf)
    );

    // send the bytes
    async_write(socket,
        boost::asio::buffer(sendbuf, sendbuf_size),
        boost::bind(&NMSConnection::sendHandler, this, _1, _2, sendbuf)
    );
};

void NMSConnection::disconnect()
    throw()
{
    boost::system::error_code ec;

    // shutdown receiver and sender end of the socket, ignore errors
    socket.shutdown(tcp::socket::shutdown_both, ec);

    // stop the io_service object
    io_service.stop();

    // try to join the thread
    try {
        worker.timed_join(boost::posix_time::millisec(threadwait_ms));

    }
    catch(...)
    {
    }

    // if the thread finished, return. otherwise try to kill the thread
    if (worker.get_id() == boost::thread::id())
        return;

    // try to interrrupt the thread
    worker.interrupt();

    // when the thread object goes out of scope, the thread detaches

    // finally notify the caller about the disconnection
    notification_callback(
        control::ProtocolNotification(
            control::ProtocolNotification::ID_DISCONNECTED
        )
    );

}


