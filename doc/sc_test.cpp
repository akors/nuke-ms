#include <iostream>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <boost/statechart/asynchronous_state_machine.hpp>
#include <boost/statechart/fifo_scheduler.hpp>

#include <boost/statechart/state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include <boost/mpl/list.hpp>


struct EventConnectRequest : boost::statechart::event<EventConnectRequest> {};

struct StateConnected;
struct StateUnconnected;

class ProtocolMachine
    : public boost::statechart::asynchronous_state_machine<ProtocolMachine,
                                                            StateUnconnected>
{
public:

    int x;

    ProtocolMachine(my_context ctx, int _x)
        : my_base(ctx), x(_x)
    {}
};

struct StateConnected
        : public boost::statechart::state<StateConnected,
                                            ProtocolMachine>
{

    StateConnected(my_context ctx)
        : my_base(ctx)
    {
        std::cout<<"Entered state Connected.\n";
        std::cout<<"x = "<<(context<ProtocolMachine>().x)<<'\n';
    }
};

struct StateUnconnected
        : public boost::statechart::state<StateUnconnected,
                                            ProtocolMachine>
{
    typedef boost::statechart::custom_reaction<EventConnectRequest> reactions;

    StateUnconnected(my_context ctx)
        : my_base(ctx)
    {
        std::cout<<"Entered state Unconnected.\n";
        std::cout<<"x = "<<(context<ProtocolMachine>().x)<<'\n';
    }

    boost::statechart::result react(const EventConnectRequest&)
    {
        std::cout<<"User requested transition\n";
        return transit<StateConnected>();
    }
};


int main()
{
    boost::statechart::fifo_scheduler<> machine_scheduler(true);

    boost::statechart::fifo_scheduler<>::processor_handle event_processor =
        machine_scheduler.create_processor<ProtocolMachine>(411);

    machine_scheduler.initiate_processor(event_processor);




    boost::thread machine_thread(
        boost::bind(&boost::statechart::fifo_scheduler<>::operator(),
                    &machine_scheduler,
                    0)
        );



    boost::intrusive_ptr<EventConnectRequest> event_ptr =
        new EventConnectRequest();

    machine_scheduler.queue_event(event_processor, event_ptr);


    machine_scheduler.terminate();
    machine_thread.join();

    return 0;
}


