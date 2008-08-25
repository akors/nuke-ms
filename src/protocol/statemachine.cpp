// statemachine.cpp

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

#include "protocol/statemachine.hpp"

using namespace nms;
using namespace protocol;

void ProtocolMachine::notificationTranslator (
    outermost_context_type& _this,
    const control::ProtocolNotification& notification
) throw()
{
    using namespace nms::control;

    switch (notification.id)
    {
        case ProtocolNotification::ID_DISCONNECTED:
        {
            // cast to a Disconnected Notification
            const DisconnectedNotification& notif =
                static_cast<const DisconnectedNotification&>(notification);

            // post the event to the State Machine
            _this.post_event(EventDisconnected(notif.msg));

            break;
        }

        case ProtocolNotification::ID_RECEIVED_MSG:
        {
            const ReceivedMsgNotification& notif =
                static_cast<const ReceivedMsgNotification&> (notification);

            // post the event to the State Machine
            _this.post_event(EventRcvdMsg(notif.msg));


            break;
        }


        case ProtocolNotification::ID_CONNECT_REPORT:
        {
            const RequestReport& rprt =
                static_cast<
                    const ReportNotification<
                                ProtocolNotification::ID_CONNECT_REPORT>&
                            >
                            (notification);

            _this.post_event(EventConnectReport(rprt.failure_reason));

            break;
        }


        default:
            /* nothing */;

    }


}


StateConnected::StateConnected()
{
    std::cout<<"We are connected!\n";
}

boost::statechart::result StateConnected::react(const EventSendMsg& msg)
{
    std::cout<<"Sending message: ";
    std::cout<<std::string(msg.parm.begin(), msg.parm.end())<<'\n';

    outermost_context().connection->send(msg.parm);

    return discard_event();
}

boost::statechart::result StateConnected::react(const EventRcvdMsg& msg)
{
    context<ProtocolMachine>()
        .notification_callback(
            control::ReceivedMsgNotification
                (msg.parm)
            );

    return discard_event();
}


boost::statechart::result StateConnected::react(const EventDisconnected& evt)
{
    context<ProtocolMachine>()
        .notification_callback(
                control::DisconnectedNotification(evt.parm)
            );

    // delete the Connection object, therefore closing the connection
    // and terminating the thread
    outermost_context().connection.reset();

    return transit<StateUnconnected>();
}

boost::statechart::result StateConnected::react(const EventDisconnectRequest&)
{

    //ask the Connection object to disconnect itself
    outermost_context().connection->disconnect();
}


boost::statechart::result
StateTryingConnect::react(const EventConnectReport& rprt)
{
    if (rprt.parm.empty())
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
                    (L"Sorry, connection failed: " + rprt.parm)
                );

        // delete the Connection object, therefore closing the connection
        // and terminating the thread
        outermost_context().connection.reset();

        return transit<StateUnconnected>();
    }
}


boost::statechart::result
StateTryingConnect::react(const EventDisconnected& evt)
{
    context<ProtocolMachine>()
        .notification_callback(
                control::DisconnectedNotification(evt.parm)
            );

    // delete the Connection object, therefore closing the connection
    // and terminating the thread
    outermost_context().connection.reset();

    return transit<StateUnconnected>();
}


boost::statechart::result
StateTryingConnect::react(const EventDisconnectRequest&)
{

    //ask the Connection object to disconnect itself
    outermost_context().connection->disconnect();

    return transit<StateUnconnected>();
}



boost::statechart::result StateIdle::react(const EventConnectRequest& request)
{
    // create an object of type NMSConnection
    outermost_context().connection.reset(
        new NMSConnection(
            request.parm,
            boost::bind(&ProtocolMachine::notificationTranslator,
                boost::ref(outermost_context()),
                _1
            )
        )
    );


    return transit<StateTryingConnect>();
}

