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

#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
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

    virtual const char* what() const throw()
    { return std::runtime_error::what(); }

    virtual ~ProtocolError() throw()
    { }
};

/** Client Communication Protocol.
*/
class NMSProtocol
{
    typedef boost::asio::ip::tcp tcp;

    class ResourcesSafe
    {    
        boost::recursive_mutex io_mutex;
        
        // boost::asio::io_service io_service;        
        boost::asio::ip::tcp::socket socket;
    
        
        /** RAII Proxy class through which all treasures can be accessed.
        * On construction, this class acquires a mutex and releases this 
        * mutex on destruction.
        */
        template <typename GuardedT>
        struct GuardedResource
        {
            GuardedT& treasure;            
            boost::lock_guard<boost::recursive_mutex> treasure_guard;
            boost::recursive_mutex& guard_mutex;
                
            /** Copy constructible 
            * Transfers the ownership of the lock from other to *this.
            */
            GuardedResource(const GuardedResource& other)
                : treasure_guard(other.guard_mutex, boost::adopt_lock),
                guard_mutex(other.guard_mutex), treasure(other.treasure)
            {
            }
            
            GuardedResource(boost::recursive_mutex& _guard_mutex, GuardedT& _treasure)
                : guard_mutex(_guard_mutex), treasure_guard(_guard_mutex),
                    treasure(_treasure)
            {}
            
            operator GuardedT&()
            {
                return treasure;
            }            
            
        };
        
    public:    
        
        //typedef GuardedResource<boost::asio::io_service> GuardedIOService;
        typedef GuardedResource<boost::asio::ip::tcp::socket> GuardedSocket;
    
        ResourcesSafe(boost::asio::io_service& _io_service)
            : socket(_io_service)
        { }

#if 0
        GuardedIOService get_io_service()
        {
            return GuardedIOService(io_mutex, io_service);
        }
#endif
        
        GuardedSocket get_socket()
        {
            return GuardedSocket(io_mutex, socket);
        }
        
    };

    
    class Worker
    {
        // wait for the thread 3 seconds
        enum { threadwait_ms = 3000 };
    
        ResourcesSafe& resources_safe;
        boost::asio::io_service& io_service; 
        
        boost::function1<void, const ProtocolNotification&> notification_callback;
        
        boost::thread waiting_thread;
        
        
        void handleTimeout(boost::system::error_code& error)
        {            
            notification_callback(ReceivedMsgNotification(L"The bell rang!"));
        }
        
    public:
    
        Worker(ResourcesSafe& safe, boost::asio::io_service& _io_service,
             boost::function1<void, const ProtocolNotification&> notf_callback)
             
            : resources_safe(safe), io_service(_io_service),
                notification_callback(notf_callback), 
                waiting_thread(boost::ref(*this))
        { }
        
        void operator()()
        {
            boost::asio::deadline_timer t(io_service, 
                boost::posix_time::seconds(5));                
            t.async_wait(boost::bind(&Worker::handleTimeout, this, _1));
            
            io_service.run();
        }
        
        /** Destructor. Kills anything that blocks. */
        ~Worker()
        {
            try {
            io_service.stop();
            io_service.reset();            
           
            waiting_thread.timed_join(boost::posix_time::millisec(threadwait_ms));
          
            // if the waiting failed, interrupt and then detach the thread
            waiting_thread.interrupt();
            waiting_thread.detach();
            } catch(...) {}
        }
        
    };
    
    boost::function1<void, const ProtocolNotification&> notification_callback;    
    boost::scoped_ptr<Worker> worker;
    
    // the io_service object is  thread safe.
    boost::asio::io_service io_service; 
    ResourcesSafe resources_safe;
public:

    NMSProtocol(const boost::function1<void, const ProtocolNotification&>&
                _notification_callback)
        : io_service(), resources_safe(io_service), 
          notification_callback(_notification_callback)
    {}

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

    ~NMSProtocol()
    {
        worker.reset();
    }
};


#endif /*PROTOCOL_HPP_*/
