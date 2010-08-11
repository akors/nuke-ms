// control.hpp

/*
 *   nuke-ms - Nuclear Messaging System
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


/** @file control.hpp
* @brief Application Control and Management
*
* This file contains the Application management. Commands and notifications that
* come from the components are processed and the appropriate actions are made.
*
* @author Alexander Korsunsky
*/



/** @defgroup app_ctrl Application Control */




#ifndef CONTROL_HPP_INCLUDED
#define CONTROL_HPP_INCLUDED

#include <iostream>

#include <stdexcept>

#include <boost/bind.hpp>

#include "bytes.hpp"
#include "control/notifications.hpp"

namespace nuke_ms
{
namespace control
{


/** Application Control Class.
* @ingroup app_ctrl
* This class controls the whole Application. It provides Callback functions
* that can be used by the components to report events. These callback functions
* determine the actions to be taken.
*
* @tparam GuiT Class of the GUI object.
* @tparam ProtocolT Class of the Connection Protocol object.
*
* @note Copy construction and assignment are not allowed.
*/
template <typename GuiT, typename ProtocolT>
class AppControl
{
    GuiT gui; /**< The GUI object */
    ProtocolT protocol; /**< The Communication protocol object */

private:

    AppControl& operator= (const AppControl&); /**<  not assignable */
    AppControl(const AppControl&); /**<   not copyable */


    /** Shut down the application. */
    void close() throw() { gui.close(); }

public:
    /** Constructor.
    * Registers the handleCommand callback with the gui object.
    *
    * Throws the same exceptions the constructors of the template arguments throw.
    */
    AppControl()
    :  protocol(boost::bind(&AppControl::handleNotification, this, _1))
    {
        // this one's for us
        gui.connectExitApp(boost::bind(&AppControl::close, this));

        // thread gui signals to protocol slots
        gui.connectConnectTo(boost::bind(&ProtocolT::connect_to,&protocol,_1));
        gui.connectSendMessage(boost::bind(&ProtocolT::send, &protocol, _1));
        gui.connectDisconnect(boost::bind(&ProtocolT::disconnect,&protocol));
    }


    /** Handle a Protocol Notification.
    * This function should be used as a callback by the Protocol, when the
    * Protocol wants to report an event.
    * @param notification The notification
    */
    void handleNotification(const ProtocolNotification& notification) throw();

    /** Return a pointer to the GUI.
    * The main Application might need to access the GUI directly. This is
    * possible by calling this function.
    */
    GuiT* getGui() throw() { return &gui; }
};


template <typename GuiT, typename ProtocolT>
void AppControl<GuiT, ProtocolT>::handleNotification
                                    (const ProtocolNotification& notification)
    throw()
{

    switch (notification.id)
    {
        case ProtocolNotification::ID_DISCONNECTED:
        {
            const DisconnectedNotification& msg =
                static_cast<const DisconnectedNotification&> (notification);

            gui.printMessage(L"*  Disconnected. Reason: " + msg.msg);
            break;
        }

        case ProtocolNotification::ID_RECEIVED_MSG:
        {
            const ReceivedMsgNotification& msg =
                static_cast<const ReceivedMsgNotification&> (notification);

            gui.printMessage(L">> " + msg.msg);
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


            if (rprt.successful)
                gui.printMessage(L"*  Connecting succeeded.");
            else
            {
                gui.printMessage(L"*  Connecting failed: " + rprt.failure_reason);
            }
            break;
        }

        case ProtocolNotification::ID_SEND_REPORT:
        {
            const SendReport& rprt =
                static_cast<const SendReport&>(notification);

            if (rprt.successful)
                ;// nothing ...
            else
            {
                gui.printMessage(L"*  Failed to send message: " +
                    rprt.failure_reason);
            }
            break;
        }


        default:
            // should not happen
            gui.printMessage(L"ERROR: Invalid Protocol Notification!");

    }
}


} // namespace control

} // namespace nuke_ms



#endif // ifndef CONTROL_HPP_INCLUDED

