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

#include <cassert>
#include <string>

#include <boost/shared_ptr.hpp>



// incomplete type declaration
class AppControl;
    

/** Base class for making an application part controllable.
* @ingroup app_ctrl
*
* Derive from this base class to make the deriving class controllable.
*/
class Controllable
{
    
protected:
    AppControl* const app_control;   
    bool app_ctrl_set;

public:
    /** Bind an Application Control Object to this instance.
    * This function can be used to register an AppControl object in this object.
    * Normally, this function is called by the constructor of AppControl.
    * <br>
    * @warning Call  this function only ONCE with an object. Calling it the 
    * second time will result in an Assertion.
    * @see AppControl
    * 
    */
    void setAppControl(AppControl* _app_control);

	/** Constructor */
    Controllable()
       : app_control( NULL ), app_ctrl_set(false)
    {}
    
};
    
    
/** Abstract Base class that represents a communication protocol.
 * @ingroup proto
 * @ingroup app_ctrl
 * 
 * This class provides basic methods for managing connections and sending data.
 * To interface a protocol class with this application, you have to derive a 
 * class from this class, and pass a pointer to an object to the Constructor of 
 * AppControl. The app_control member shall then be set using the 
 * setAppControlFunction of the base class.
 * 
 * @see AppControl
 */
 class AbstractProtocol : public Controllable
 {
    /** Connect to a remote site.
     * @param id The string representation of the address of the remote site
     */
 	virtual void connect_to(const std::wstring& id) = 0;
    
    /* Send message to connected remote site.
     * @param msg The message you want to send
     */
    virtual void send(const std::wstring& id) = 0;
    
    /** Disconnect from the remote site.
     * 
     */
    virtual void disconnect() = 0;
    
    
    virtual bool is_connected() = 0;
 	
    /** AppControl is a friend, because it knows what to do */
    friend class AppControl;
 };



/** Abstract Base class that represents a Graphical User Interface in this
* application.
* @ingroup gui
* @ingroup app_ctrl
* This class provides basic Input/Output facilities between the user (actually
* the deriving GUI class) and the Application Control Class. <br>
* To interface a GUI with this application, you have to derive a class from
* this class, and pass a pointer to an object to the Constructor of AppControl.
* The app_control member shall then be set using the setAppControlFunction
* of the base class.
*
* @see AppControl
*/
class AbstractGui : public Controllable
{
    
private:


	/** Print a message to the GUI.
	*
	* Override this abstract member function with an appropriate 
	* implementation that prints a message to the GUI.
	*
	* @param str Message to be printed
	*/
	virtual void printMessage(const std::wstring& str) = 0;

	/** Shut down the GUI:
	*
	* Override this abstract member function with an appropriate 
	* implementation that prints a message to the GUI.	
	* @post The GUI is shut down. (e.g. the window is closed.)
	*/
	virtual void close() = 0;


    /** AppControl is a friend, because it knows what to do */
    friend class AppControl;
};




/** Application Control Class.
* @ingroup app_ctrl
* This class controls the whole Application. It provides Member funcions
* which control each part of the application. <br>
* When creating an instance of this class you have to bind (register) a gui Object 
* that is derived from AbstractGui by passing a pointer to it. 
* By doing so, you also register the instance in the gui object.
* 
* @note Copy construction and assignment are not allowed.
*/
class AppControl 
{
    AbstractGui* const gui;
    AbstractProtocol* const protocol;
    
private:
    // not assignable, not copyable
    AppControl& operator= (const AppControl&);
    AppControl(const AppControl&);
        
public:
	/** Constructor.
	*
	* Creates an object, registers the gui object and registers this in the
	* gui object.
	*
	* @param _gui The gui object you want to bind to this object.
	* @see AbstractGui
	*/  
	AppControl(AbstractGui* _gui, AbstractProtocol* _protocol)
	: gui(_gui), protocol(_protocol)
	{
		gui->setAppControl(this);
		protocol->setAppControl(this);
	}
	
	/** Print a message to the screen securely
	* 
	* @param the message you want to print
	*/
	void printMessage(const std::wstring& msg)
	{
		gui->printMessage(msg);
	}
	
	void close()
	{
		gui->close();
	}
};

#endif // ifndef CONTROL_HPP_INCLUDED

