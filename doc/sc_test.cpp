#include <iostream>

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/mpl/list.hpp>





struct EventConnectRequest : boost::statechart::event<EventConnectRequest> {};
struct EventConnectReport : boost::statechart::event<EventConnectRequest>
{
    bool success;

    EventConnectReport(bool _success)
        : success(_success)
    {}

};

struct EventRcvdMsg : boost::statechart::event<EventRcvdMsg>
{
    std::string msg;
    EventRcvdMsg(const std::string& _msg)
        : msg(_msg)
    { }
};

struct EventDisconnect : boost::statechart::event<EventDisconnect> {};





struct StateUnconnected;
struct StateConnected;



class ProtocolMachine
    : public boost::statechart::state_machine<ProtocolMachine, StateUnconnected>
{

};


struct StateIdle;
struct StateTryingConnect;

struct StateUnconnected
    : public boost::statechart::simple_state<StateUnconnected,
                                                ProtocolMachine,
                                                StateIdle>
{

    StateUnconnected()
    { std::cout<<"We are not connected.\n"; }

};


struct StateIdle
    : public boost::statechart::simple_state<StateIdle,
                                                StateUnconnected>
{
    typedef boost::statechart::custom_reaction< EventConnectRequest > reactions;

    StateIdle()
    {
        std::cout<<"Machine in Connected/Idle\n";
    }

    boost::statechart::result react(const EventConnectRequest&)
    {
        return transit<StateTryingConnect>();
    }
};

struct StateTryingConnect
    : public boost::statechart::simple_state<StateTryingConnect,
                                                StateUnconnected>
{
    typedef boost::statechart::custom_reaction< EventConnectReport > reactions;

    StateTryingConnect()
    {
        std::cout<<"Trying to connect\n";
    }

    boost::statechart::result react(const EventConnectReport& rprt)
    {
        if (rprt.success)
        {
            std::cout<<"Connection successful! Changin to connected\n";
            return transit<StateConnected>();
        }
        else
        {
            std::cout<<"Sorry, connection failed.\n";
            return transit<StateUnconnected>();
        }
    }

};


struct StateConnected
    : public boost::statechart::simple_state<StateConnected,
                                                ProtocolMachine>
{
    StateConnected()
    {
        std::cout<<"We are connected!\n";
    }

#if 0
    ~StateConnected()
    { std::cout<<"Leaving connected.\n"; }
#endif

    typedef boost::mpl::list<
                boost::statechart::custom_reaction< EventRcvdMsg >,
                boost::statechart::transition<EventDisconnect, StateUnconnected>
                >
        reactions;


    boost::statechart::result react(const EventRcvdMsg& msg)
    {
        std::cout<<"Received mgs: "<<msg.msg<<'\n';
        return discard_event();
    }
};


int main()
{
    ProtocolMachine protocol_machine;
    protocol_machine.initiate();

    std::cout<<"\n-- Sending EventConnectRequest\n";
    protocol_machine.process_event(EventConnectRequest());

    std::cout<<"\n-- Sending Successful connection\n";
    protocol_machine.process_event(EventConnectReport(true));

    std::cout<<"\n-- Sending EventRcvdMsg\n";
    protocol_machine.process_event(EventRcvdMsg("Hello World!"));

    std::cout<<"\n-- Sending Disconnect\n";
    protocol_machine.process_event(EventDisconnect());

    return 0;
}


