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

#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>


#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include <boost/function.hpp>

#include "control.hpp"

namespace nms 
{
namespace protocol
{

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






struct EventConnectRequest : boost::statechart::event<EventConnectRequest> 
{
    std::wstring where;
    
    EventConnectRequest(const std::wstring& _where)
        : where(_where)
    {}
};


struct EventConnectReport : boost::statechart::event<EventConnectRequest>
{
    bool success;

    EventConnectReport(bool _success)
        : success(_success)
    {}

};

struct EventRcvdMsg : boost::statechart::event<EventRcvdMsg>
{
    std::wstring msg;
    EventRcvdMsg(const std::wstring& _msg)
        : msg(_msg)
    { }
};


struct EventSendMsg : boost::statechart::event<EventSendMsg>
{
    std::wstring msg;
    EventSendMsg(const std::wstring& _msg)
        : msg(_msg)
    { }
};

struct EventDisconnect : boost::statechart::event<EventDisconnect> 
{};




struct StateUnconnected;
struct StateConnected;


struct ProtocolMachine
    : public boost::statechart::state_machine<ProtocolMachine, StateUnconnected>
{
    boost::function1 <void, const control::ProtocolNotification&> 
        notification_callback;
    
    ProtocolMachine(const boost::function1
                        <void, const control::ProtocolNotification&>&
                        _notification_callback)
        : notification_callback(_notification_callback)
    {}
};





struct StateIdle;
struct StateTryingConnect;

struct StateUnconnected
    : public boost::statechart::simple_state<StateUnconnected,
                                                ProtocolMachine,
                                                StateIdle>
{ };


struct StateIdle
    : public boost::statechart::simple_state<StateIdle,
                                                StateUnconnected>
{
    typedef boost::statechart::custom_reaction< EventConnectRequest > reactions;


    boost::statechart::result react(const EventConnectRequest& request)
    {
        std::cout<<"Trying to connect to "<<std::string(request.where.begin(), request.where.end())<<"\n";
        return transit<StateTryingConnect>();
    }
};

struct StateTryingConnect
    : public boost::statechart::simple_state<StateTryingConnect,
                                                StateUnconnected>
{
    typedef boost::statechart::custom_reaction< EventConnectReport > reactions;

    boost::statechart::result react(const EventConnectReport& rprt)
    {
        if (rprt.success)
        {
            context<ProtocolMachine>()
                .notification_callback(
                    control::ReportNotification
                        <control::ProtocolNotification::ID_CONNECT_REPORT>()
                    );
                    
            return transit<StateConnected>();
        }
        else
        {
            context<ProtocolMachine>()
                .notification_callback(
                    control::ReportNotification
                        <control::ProtocolNotification::ID_CONNECT_REPORT>
                        (L"Sorry, connection failed.\n")
                    );
        
            return transit<StateUnconnected>();
        }
    }

};


struct StateConnected
    : public boost::statechart::simple_state<StateConnected,
                                                ProtocolMachine>
{
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EventSendMsg >,
        boost::statechart::custom_reaction< EventRcvdMsg >,
        boost::statechart::custom_reaction<EventDisconnect>
    > reactions;

    StateConnected()
    {
        std::cout<<"We are connected!\n";
    }
    
    ~StateConnected()
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ProtocolNotification
                    (control::ProtocolNotification::ID_DISCONNECTED)
                );
    }
    


    
    boost::statechart::result react(const EventSendMsg& msg)
    {
        std::cout<<"Sending message: "<<std::string(msg.msg.begin(), msg.msg.end())<<'\n';
        return discard_event();
    }

    boost::statechart::result react(const EventRcvdMsg& msg)
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ReceivedMsgNotification
                    (msg.msg)
                );
                
        return discard_event();
    }
    
    boost::statechart::result react(const EventDisconnect&)
    {

        return transit<StateUnconnected>();
    }

};



/** Client Communication Protocol.
*/
class NMSProtocol
{
    ProtocolMachine protocol_machine;
    
    typedef boost::asio::ip::tcp tcp;

    boost::function1<void, const control::ProtocolNotification&> 
        notification_callback;
    

    
    
public:

    NMSProtocol(const boost::function1<void, 
                                        const control::ProtocolNotification&>&
                    _notification_callback)
        : notification_callback(_notification_callback), 
            protocol_machine(_notification_callback)
    {
        protocol_machine.initiate();
    }

    /** Connect to a remote site.
     * @param id The string representation of the address of the remote site
     */
    void connect_to(const std::wstring& id)
    {       
        protocol_machine.process_event(EventConnectRequest(id));
        protocol_machine.process_event(EventConnectReport(true));
    }

    /** Send message to connected remote site.
     * @param msg The message you want to send
     */
    void send(const std::wstring& msg)
    {
        protocol_machine.process_event(EventSendMsg(msg));
    }

    /** Disconnect from the remote site.
     *
     */
    void disconnect()
    {
        protocol_machine.process_event(EventDisconnect());
    }
    
};

} // namespace protocol
} // namespace nms 

#endif /*PROTOCOL_HPP_*/
