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


/** A command to the Application Control Class.
* @ingroup ctrl_cmd
* Send this command to AppControl.
*/
struct ControlCommand
{
	
    /** The Command ID Type
    * @ingroup ctrl_cmd
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

/** Command to print a message.
* @ingroup ctrl_cmd
*/
struct Command_PrintMessage : public ControlCommand
{
	// the message you want to print
	const std::wstring msg;
	
	Command_PrintMessage(const std::wstring& str)
		: msg(str), ControlCommand(ID_PRINT_MSG)
	{ }
};

/** Command to send a message.
* @ingroup ctrl_cmd
*/
struct Command_SendMessage : public ControlCommand
{
	// the message you want to print
	const std::wstring msg;
	
	Command_SendMessage(const std::wstring& str)
		: msg(str), ControlCommand(ID_SEND_MSG)
	{ }
};

/** Command to connect to a remote site.
* @ingroup ctrl_cmd
*/
struct Command_ConnectTo : public ControlCommand
{
    const std::wstring remote_host;
    
	Command_ConnectTo(const std::wstring& str)
		: remote_host(str), ControlCommand(ID_CONNECT_TO)
	{ }
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
    
	
    void connectTo(const std::wstring& id);
	
	void sendMessage(const std::wstring& msg);

	
	void disconnect()
	{
	    protocol.disconnect();
	    this->printMessage(L"Disconnected");
	}
	
	/** Print a message to the screen securely
	* 
	* @param the message you want to print
	*/
	void printMessage(const std::wstring& msg)
	{
		gui.printMessage(msg);
	}
	
	void close()
	{
		gui.close();
	}
        
public:
	/** Constructor.
	*
	* Creates an object, registers the gui object and registers this in the
	* gui object.
	*
	* @param _gui The gui object you want to bind to this object.
	* @see AbstractGui
	*/  
	AppControl()
	: protocol(), gui(boost::bind(&AppControl::handleCommand, this, _1))
	{ }
	
	
	void handleCommand(const ControlCommand& ctrl_cmd);
};



template <typename GuiT, typename ProtocolT>
void AppControl<GuiT, ProtocolT>::connectTo(const std::wstring& id)
{	    
    try { 
        protocol.connect_to(id);
        this->printMessage(L"Connected to " + id);
    } 
    catch (const std::exception& e)
    {
        std::string errmsg(e.what());	        
	    this->printMessage(L"Failed to connect to " + id + L": " 
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
			const Command_PrintMessage& cmd_msg =
				dynamic_cast<const Command_PrintMessage&> (cmd);
			
			printMessage(cmd_msg.msg);
            break;
        }
        
        case ControlCommand::ID_SEND_MSG:
        {
            if ( !protocol.is_connected())
                return;
        
			const Command_SendMessage& cmd_msg =
				dynamic_cast<const Command_SendMessage&> (cmd);
			
			sendMessage(cmd_msg.msg);
            break;
        }
            
        case ControlCommand::ID_CONNECT_TO:
        {
		    const Command_ConnectTo& cmd_cnt =
		        dynamic_cast<const Command_ConnectTo&> (cmd);
		        
	        connectTo(cmd_cnt.remote_host);
            break;
        }
            
        default:
            printMessage(L"Invalid command!");
            break;
            
    }
}

#endif // ifndef CONTROL_HPP_INCLUDED

