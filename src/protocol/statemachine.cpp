// statemachine.cpp

/*
 *   NMS - Nuclear Messaging System
 *   Copyright (C) 2008, 2009  Alexander Korsunsky
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

using namespace nms::protocol;

ProtocolMachine::ProtocolMachine(my_context ctx,
                nms::control::notif_callback_t _notification_callback)
    : my_base(ctx), notification_callback(_notification_callback)
{}



StateWaiting::StateWaiting(my_context ctx)
    : my_base(ctx)
{
    std::cout<<"Waiting to be connected\n";
}

boost::statechart::result StateWaiting::react(const EvtConnectRequest & evt)
{
    return transit< StateNegotiating >();
}

boost::statechart::result StateWaiting::react(const EvtSendMsg& evt)
{
    context<ProtocolMachine>()
        .notification_callback(
            control::SendReport(evt.msg, std::wstring(L"Not Connected."))
            );

    return discard_event();
}


StateNegotiating::StateNegotiating(my_context ctx)
    : my_base(ctx)
{
    std::cout<<"Negotiating\n";

    // send positive connection report for debugging purposes
    post_event(
        EvtConnectReport(true, std::wstring(L"Connection attempt succeeded."))
    );
}

boost::statechart::result StateNegotiating::react(const EvtSendMsg& evt)
{
    // we dont send messages to nowhere
    context<ProtocolMachine>()
        .notification_callback(
            control::SendReport(evt.msg, std::wstring(L"Not yet Connected."))
            );

    return discard_event();
}

boost::statechart::result StateNegotiating::react(const EvtConnectReport& evt)
{
    // change state according to the outcome of a connection attempt
    if ( evt.success )
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
                    <control::ProtocolNotification::ID_CONNECT_REPORT>(
                        std::wstring(L"Connection Failed")
                    )
                );

        return transit<StateWaiting>();
    }
}

boost::statechart::result StateNegotiating::react(const EvtDisconnectRequest&)
{
    return transit<StateWaiting>();
}

StateConnected::StateConnected(my_context ctx)
    : my_base(ctx)
{
    std::cout<<"Connected\n";
}


boost::statechart::result StateConnected::react(const EvtDisconnectRequest&)
{
    return transit<StateWaiting>();
}


boost::statechart::result StateConnected::react(const EvtSendMsg& evt)
{
#if 1
    std::wcout<<L" << "<<evt.msg<<L'\n';
#endif

    return discard_event();
}
