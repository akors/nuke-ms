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
#include <string>

#include <boost/bind.hpp>

#include "control/notifications.hpp"
#include "control/commands.hpp"

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

    /** Connect to a remote site.
    * The Protocol is requested to connect to a remote site, that is uniquely
    * identified by the string id.
    *
    * @param id The string that identifies the remote site. This string must be
    * understood by the protocol.
    */
    void connectTo(const std::wstring& id)
        throw();

    /** Send a message to the site that is connected.
    * If the protocol is not connected to a remote site or failed to send
    * the message, an error message is displayed.
    * @param msg The message you want to send
    */
    void sendMessage(const std::wstring& msg)
        throw();

    /** Disconnect from a remote site, if connected */
    void disconnect()
        throw()
    {
        try {
            protocol.disconnect();
        }
        catch (const std::exception& e) {
            const char* errmsg = e.what();
            printMessage(L"*  Failed to disconnect: " +
                            std::wstring(errmsg, errmsg+strlen(errmsg)));
        }
    }

    /** Print a message to the screen securely
    *
    * @param msg the message you want to print
    */
    void printMessage(const std::wstring& msg)
        throw()
    {
        try {
            gui.printMessage(msg);
        }
        catch (const std::exception& e)
        {
            std::cerr<<"ERROR: Failed to print message: "<<e.what()<<'\n';
        }
    }


    /** Shut down the application. */
    void close()
        throw()
    {
        gui.close();
    }

public:
    /** Constructor.
    * Registers the handleCommand callback with the gui object.
    *
    * Throws the same exceptions the constructors of the template arguments throw.
    */
    AppControl()
    :  gui(boost::bind(&AppControl::handleCommand, this, _1)),
       protocol(boost::bind(&AppControl::handleNotification, this, _1))
    { }

    /** Handle a command.
    * This function should be used as a callback for the GUI.
    * Commands that are passed in are evaluated. If requested action can not
    * be performed, an error message is displayed.
    * @param ctrl_cmd The command that
    */
    void handleCommand(const ControlCommand& ctrl_cmd) throw();


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
    GuiT* getGui()
        throw()
    { return &gui; }
};



template <typename GuiT, typename ProtocolT>
void AppControl<GuiT, ProtocolT>::connectTo(const std::wstring& id)
    throw()
{
    try {
        protocol.connect_to(id);
    }
    catch (const std::exception& e)
    {
        std::string errmsg(e.what());
        this->printMessage(L"* Failed to connect to " + id + L": "
                          + std::wstring(errmsg.begin(), errmsg.end()) );
    }
}

template <typename GuiT, typename ProtocolT>
void AppControl<GuiT, ProtocolT>::sendMessage(const std::wstring& msg)
    throw()
{
    try {
        protocol.send(msg);
    }
    catch (const std::exception& e)
    {
        std::string errmsg(e.what());
        this->printMessage(L"Failed to send message: "
                            + std::wstring(errmsg.begin(), errmsg.end()));
    }
}

template <typename GuiT, typename ProtocolT>
void AppControl<GuiT, ProtocolT>::handleCommand(const ControlCommand& cmd)
    throw()
{
    // determine actions depending on the id of the command
    switch ( cmd.id )
    {

        case ControlCommand::ID_EXIT:
            close();
            break;

        case ControlCommand::ID_DISCONNECT:
            disconnect();
            break;

        case ControlCommand::ID_PRINT_MSG:
        {
            const MessageCommand<ControlCommand::ID_PRINT_MSG>& cmd_msg =
                static_cast<const MessageCommand<ControlCommand::ID_PRINT_MSG>&> (cmd);

            printMessage(L"<< " + cmd_msg.msg);
            break;
        }

        case ControlCommand::ID_SEND_MSG:
        {
            const MessageCommand<ControlCommand::ID_SEND_MSG>& cmd_msg =
                static_cast<const MessageCommand<ControlCommand::ID_SEND_MSG>&> (cmd);

            sendMessage(cmd_msg.msg);
            break;
        }

        case ControlCommand::ID_CONNECT_TO:
        {
            const MessageCommand<ControlCommand::ID_CONNECT_TO>& cmd_cnt =
                static_cast<const MessageCommand<ControlCommand::ID_CONNECT_TO>&> (cmd);

            connectTo(cmd_cnt.msg);
            break;
        }

        default:
            printMessage(L"Invalid command!");
            break;

    }
}

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

            printMessage(L"*  Disconnected. Reason: " + msg.msg);
            break;
        }

        case ProtocolNotification::ID_RECEIVED_MSG:
        {
            const ReceivedMsgNotification& msg =
                static_cast<const ReceivedMsgNotification&> (notification);

            printMessage(L">> " + msg.msg);
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
                printMessage(L"*  Connecting succeeded.");
            else
            {
                printMessage(L"*  Connecting failed: " + rprt.failure_reason);
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
                printMessage(L"*  Failed to send message \"" +
                                rprt.message + L"\": " + rprt.failure_reason);
            }
            break;
        }


        default:
            // should not happen
            printMessage(L"ERROR: Invalid Protocol Notification!");

    }
}


} // namespace control

} // namespace nuke_ms



#endif // ifndef CONTROL_HPP_INCLUDED

