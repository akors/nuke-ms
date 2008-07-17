// control.hpp

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


/** @mainpage
*
* The Nuclear Messaging System is intended as a cross- plattform,
* highly portable instant messenger.
*/




/** @defgroup app_ctrl Application Control */
/** @defgroup  ctrl_cmd Control Commands
*@ingroup app_ctrl */



#ifndef CONTROL_HPP_INCLUDED
#define CONTROL_HPP_INCLUDED




#include <stdexcept>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>


namespace nms
{
namespace control
{

/** A command to the Application Control Class.
* @ingroup ctrl_cmd
* Send this command to AppControl.
*/
struct ControlCommand
{


    /** The Command ID Type
    */
    enum command_id_t {
        ID_INVALID = 0, /**< Invalid Command */
        ID_EXIT, /**< user asked to exit */
        ID_PRINT_MSG, /**< print a message */
        ID_SEND_MSG, /**< Send the message to the remote site */
        ID_CONNECT_TO, /**< connect to a remote site */
        ID_DISCONNECT, /**< disconnect from a remote site */

        /** Number of command identifiers.
        * As the first command (ID_INVALID) starts from 0,
        * this element specifies how many commands there are.
        */
        ID_NUM_COMMANDS
    };

    /** The command ID*/
    const command_id_t id;

    /** Constructor. */
    ControlCommand(const command_id_t& _id)
        : id(_id)
    {}

    virtual ~ControlCommand()
    {}
};

/** This template is for all commands that are used to transfer one string as a
* message.
*/
template <ControlCommand::command_id_t CMD_ID>
struct MessageCommand : public ControlCommand
{
    const std::wstring msg;

    MessageCommand(const std::wstring& str)
        : msg(str), ControlCommand(CMD_ID)
    { }
};



/** A notification from the Communication protocol regarding an event */
struct ProtocolNotification
{
    /** The type of operation that is reported about.
    */
    enum notification_id_t {
        ID_CONNECT_REPORT,
        ID_SEND_REPORT,
        ID_RECEIVED_MSG,
        ID_DISCONNECTED
    };

    const notification_id_t id;

    /** Construct a Report denoting failure */
    ProtocolNotification(notification_id_t _id)
        : id(_id)
    {}

    virtual ~ProtocolNotification()
    {}
};


struct ReceivedMsgNotification : public ProtocolNotification
{
    const std::wstring msg;

    ReceivedMsgNotification(const std::wstring& _msg)
        : msg(_msg), ProtocolNotification(ID_RECEIVED_MSG)
    {}
};


/** This class represents a positive or negative reply to a requested operation.
*/
struct RequestReport
{
    const bool successful;
    const std::wstring failure_reason;

    /** Construct a positive report */
    RequestReport()
        : successful(true)
    {}

    /** Construct a negative report */
    RequestReport(const std::wstring& reason)
        : successful(false), failure_reason(reason)
    {}

    virtual ~RequestReport()
    {}
};

/** A generic notification that involves reporting about Success or failure */
template <ProtocolNotification::notification_id_t NOTIF_ID>
struct ReportNotification : public ProtocolNotification, public RequestReport
{
    /** Generate a successful report */
    ReportNotification()
        : ProtocolNotification(NOTIF_ID), RequestReport()
    {}

    /** Generate a negative report */
    ReportNotification(const std::wstring& reason)
        : ProtocolNotification(NOTIF_ID), RequestReport(reason)
    {}
};




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
    GuiT gui;
    ProtocolT protocol;

    // not assignable, not copyable
    AppControl& operator= (const AppControl&);
    AppControl(const AppControl&);

    /** Connect to a remote site.
    * The Protocol is requested to connect to a remote site, that is uniquely
    * identified by the string id.
    *
    * @param id The string that identifies the remote site. This string must be
    * understood by the protocol.
    */
    void connectTo(const std::wstring& id);

    /** Send a message to the site that is connected.
    * If the protocol is not connected to a remote site or failed to send
    * the message, an error message is displayed.
    * @param msg The message you want to send
    */
    void sendMessage(const std::wstring& msg);

    /** Disconnect from a remote site, if connected */
    void disconnect()
    {
        protocol.disconnect();
    }

    /** Print a message to the screen securely
    *
    * @param the message you want to print
    */
    void printMessage(const std::wstring& msg)
    {
        gui.printMessage(msg);
    }

    /** Shut down the application. */
    void close()
    {
        gui.close();
    }

public:
    /** Constructor.
    * Registers the handleCommand callback with the gui object.
    */
    AppControl()
    : protocol(boost::bind(&AppControl::handleNotification, this, _1)),
        gui(boost::bind(&AppControl::handleCommand, this, _1))
    { }

    /** Handle a command.
    * This function should be used as a callback for the GUI.
    * Commands that are passed in are evaluated. If requested action can not
    * be performed, an error message is displayed.
    * @param ctrl_cmd The command that
    */
    void handleCommand(const ControlCommand& ctrl_cmd);


    /** Handle a Protocol Notification.
    * This function should be used as a callback by the Protocol, when the
    * Protocol wants to report an event.
    * @param rprt The report
    */
    void handleNotification(const ProtocolNotification& notification);
};



template <typename GuiT, typename ProtocolT>
void AppControl<GuiT, ProtocolT>::connectTo(const std::wstring& id)
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
                dynamic_cast<const MessageCommand<ControlCommand::ID_PRINT_MSG>&> (cmd);

            printMessage(L"<< " + cmd_msg.msg);
            break;
        }

        case ControlCommand::ID_SEND_MSG:
        {
            const MessageCommand<ControlCommand::ID_SEND_MSG>& cmd_msg =
                dynamic_cast<const MessageCommand<ControlCommand::ID_SEND_MSG>&> (cmd);

            sendMessage(cmd_msg.msg);
            break;
        }

        case ControlCommand::ID_CONNECT_TO:
        {
            const MessageCommand<ControlCommand::ID_CONNECT_TO>& cmd_cnt =
                dynamic_cast<const MessageCommand<ControlCommand::ID_CONNECT_TO>&> (cmd);

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
{

    switch (notification.id)
    {
        case ProtocolNotification::ID_DISCONNECTED:
        {
            printMessage(L"*  Disconnected.");
            break;
        }

        case ProtocolNotification::ID_RECEIVED_MSG:
        {
            const ReceivedMsgNotification& msg =
                dynamic_cast<const ReceivedMsgNotification&> (notification);

            printMessage(L">> " + msg.msg);
            break;
        }

        case ProtocolNotification::ID_CONNECT_REPORT:
        {
            const RequestReport& rprt =
                dynamic_cast<const RequestReport&> (notification);

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
            const RequestReport& rprt =
                dynamic_cast<const RequestReport&> (notification);

            if (rprt.successful)
                ;// nothing ...
            else
            {
                printMessage(L"*  Failed to send message: " + 
                                rprt.failure_reason);
            }
            break;
        }
        default:
            // should not happen
            printMessage(L"ERROR: Invalid Protocol Notification!");

    }
}


} // namespace control

} // namespace nms



#endif // ifndef CONTROL_HPP_INCLUDED

