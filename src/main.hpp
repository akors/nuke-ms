// main.hpp

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

/** @file main.hpp
* @brief Application entry point and graphical user interface
*
* @author Alexander Korsunsky
*
*/

/** @defgroup gui Graphical User Interface */

#ifndef MAIN_HPP_INCLUDED
#define MAIN_HPP_INCLUDED

#include <boost/function.hpp>

#include <wx/wx.h>
#include <wx/app.h>
#include <wx/frame.h>
#include <wx/menu.h>

#include "control.hpp"
#include "protocol.hpp"

/** Proportions for the window.
* @ingroup gui
* This struct contains members that are used by MainFrame to determine the size
* and proportions of the window.
*
* @see MainFrame
*/
struct WindowScales
{
	unsigned int border_width; /**< Border width*/

	wxSize displaybox_size; /**< size of the Output text box */
	wxSize inputbox_size; /**< size of the input text box*/

    /** How many parts of the window size will the output text box use*/
	unsigned int displaybox_proportion;
    
    /** How many parts of the window size will the input text box use*/
	unsigned int inputbox_proportion;
};



	
	


class MainFrameWrapper;


/** Main Window.
* @ingroup gui
* This class represents the main window. Construct it and a window will appear.
* <br> 
* After construction immediately call the inherited member setAppControl()
* to bind the GUI object to the application control object.
*
* @see AbstractGui::setAppControl
*/
class MainFrame : public wxFrame
{
	/** Enum for wxWidgets window identifiers.
	*
	*/
	enum { ID_INPUT_BOX = wxID_HIGHEST+1 };


	/** proportions and sizes for this window. */
	WindowScales scales;

	/** menu bar */
	wxMenuBar* menu_bar;

	/** Menu item called "File" */
	wxMenu* menu_file;

	/** Output text box*/
	wxTextCtrl* text_display_box;

	/** Input text box */
	wxTextCtrl* text_input_box;

    boost::function1<void, const ControlCommand&> commandCallback;


	/** creates and initializes menu bars*/
	void createMenuBar();

	/** creates and initializes text boxes */
	void createTextBoxes();

	/** Print a message to the Output text box.
	*
	* @param str The string you  want to print
	*/
	void printMessage(const wxString& str);

	/** Print a message to the Output text box.
	*
	* @param str The string you  want to print
	*/
	void printMessage(const std::wstring& str);



	/** Check if a string is a command
	* 
	* @param str The string you want to be checked
	* @todo Add proper checking here
	*/
	static bool isCommand(const std::wstring& str)
	{
    	return ((str.size() != 0) &&(str[0] == L'/'));
	}


	/** Parse a string and interpret it as command.
	* You might want to check if this seems like a Command befor calling this 
    * function
	* 
	* @param str The string you want to be interpreted as command
	* @return A pointer to the command that was retrieved from parsing.
	* ControlCommand::id == ID_INVALID, if the command could not be parsed.
	* @note the return value points to a heap object. Don't forget to delete it,
	* when you dont need it anymore.
	* @todo add proper parser here
	*/
	static boost::shared_ptr<ControlCommand>
	parseCommand(const std::wstring& str);    

public:
	// opening a hole for MainApp, so it can call OnEnter when 
	static boost::function1<void, wxCommandEvent&> OnEnter_callback;

    friend class MainFrameWrapper;
    
	/** Constructor.
	*
	* Initialize base class, set scales,
	* create menu bar and text boxes
	*/
	MainFrame( boost::function1<void, const ControlCommand&> _commandCallback);    

	/** Called if the user wants to quit. */
	void OnQuit(wxCommandEvent& event);

	/** Called i the user hits the return key without 
	*holding down the shift or ctrl. key
	*/
	void OnEnter(wxCommandEvent& event);    


	DECLARE_EVENT_TABLE()
};


class MainFrameWrapper
{
    MainFrame* main_frame;

public:
    MainFrameWrapper 
        (boost::function1<void, const ControlCommand&> commandCallback)
        : main_frame(NULL)
        
    { 
        main_frame = new MainFrame(commandCallback);    
    }
    
    ~MainFrameWrapper()
    {
        // MainFrame is derived from wxWindow. As such, simply deleting the 
        // object would be incorrect. Instead we have to ask it to destroy 
        // itself.
        //main_frame->Close();
    }
    
    
    void printMessage(const std::wstring& str)
    {
        main_frame->printMessage(str);
    }
    
	void close()
	{
		main_frame->Close(false);
	}

};



/** Application entry point.
* @ingroup gui
* This class represents the main thread of execution.
* 
*
*/
class MainApp : public wxApp
{


	/** Application control object*/
	AppControl<MainFrameWrapper, NMSProtocol>* app_control;
	

public:

	bool OnInit(); /**< gets called when the application starts */
	int OnExit(); /**< gets called when the application terminates*/

	/** Process a wxWidget event 
	*
	* This member function gets called whenever an event happens.
	* It overrides the base class version to get more control over the event
	* handling.
	* <br> <br>
	* It checks if the event is a key event. If it is, it checks wether 
	* the event was caused by pressing down the Return key.
	* If it was and control or Shift were not pressed, the function calls
	* MainFrame::OnEnter() and returns. <br>
	* If any of these conditions are not true, the function calls the base
	* class version of itself and returns.
	*/
	virtual bool ProcessEvent(wxEvent& event);
};


/** create a std::wstring object from a wxString.
*
* @param str The string you want to convert
* @return the output string
* @warning Probably not portable
*/
inline std::wstring wxString2wstring(const wxString& str)
{
	return std::wstring( str.wc_str(wxConvLocal) );
}


/** create a wxString object from an std::wstring. 
*
* @param str The string you want to convert
* @return the output string
* @warning Probably not portable
*/
inline wxString wstring2wxString(const std::wstring& str)
{
	return wxString( str.c_str() );
}



#endif // ifndef MAIN_HPP_INCLUDED


