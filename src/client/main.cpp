// main.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** @file main.cpp
* @brief Application entry point
*
* This file contains the main() function and launches the application.
*
* @author Alexander Korsunsky
*
*/

#include <iostream>
#include <wx/app.h>

#include "gui/gui.hpp"
#include "protocol/protocol.hpp"


using namespace nuke_ms;
using namespace gui;



/** Application entry point.
* @ingroup gui
* This class represents the main thread of execution.
*
* @todo Do semthing sensible when an exception is thrown.
*/
class MainApp : public wxApp
{
	/** Network protocol object */
	protocol::NukeMSProtocol protocol;
	
	/** Main GUI window */
	gui::MainFrame *main_frame;

public:

    bool OnInit() throw(); /**< gets called when the application starts */
    int OnExit() throw(); /**< gets called when the application terminates*/

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
    virtual bool ProcessEvent(wxEvent& event) throw();
};




////////////////////////////////////////////////////////////////////////////////
////////////////////////////// MainApp Methods //////////////////////////////
////////////////////////////////////////////////////////////////////////////////


bool MainApp::ProcessEvent(wxEvent& event)
    throw()
{
    bool baseclass_result;
    try {
        // check for key down event
        if (event.GetEventType() == wxEVT_CHAR)
        {
            // only if control and shift are NOT down, call OnEnter
            if ( static_cast<wxKeyEvent&>(event).GetKeyCode() == WXK_RETURN &&
                !static_cast<wxKeyEvent&>(event).ControlDown() &&
                !static_cast<wxKeyEvent&>(event).ShiftDown()
                )
            {
                wxCommandEvent cmd_evt(wxEVT_COMMAND_ENTER);
                main_frame->OnEnter(cmd_evt);
                return true;
            }
        }

        // dont stop processing the event in this function
        event.Skip();

        // call base class version of this function
        baseclass_result = wxEvtHandler::ProcessEvent(event);
    }

    // there isn't much to do if an exception propagated until here
    catch(const std::exception& e)
    {
        std::cerr<<"ERROR: An exception occured: "<<e.what()<<". Terminating\n";
        this->GetTopWindow()->Close();
    }
    catch(...)
    {
        std::cerr<<"ERROR: An unknown exception occured. Terminating\n";
        this->GetTopWindow()->Close();
    }

    return baseclass_result;
}



bool MainApp::OnInit()
    throw()
{
    
    try {
	// Create main_frame gui object
	main_frame = new gui::MainFrame;
	
	// connect all signals to the appropriate slots		

	// thread gui signals to protocol slots
	main_frame->connectConnectTo(
		boost::bind(&protocol::NukeMSProtocol::connect_to,&protocol,_1));
	main_frame->connectSendMessage(
		boost::bind(&protocol::NukeMSProtocol::send, &protocol, _1));
	main_frame->connectDisconnect(
		boost::bind(&protocol::NukeMSProtocol::disconnect,&protocol));

	// thread protocol signals to gui slots
	protocol.connectRcvMessage(
		boost::bind(&gui::MainFrame::slotReceiveMessage,main_frame,_1));
	protocol.connectConnectionStatusReport(
		boost::bind(&gui::MainFrame::slotConnectionStatusReport,main_frame,_1));
	protocol.connectSendReport(
		boost::bind(&gui::MainFrame::slotSendReport,main_frame,_1));
		
		
		
    } // say goodbye if initialization failed
    catch(const std::exception& e)
    {
        std::cerr<<"ERROR: An exception occured: "<<e.what()<<". Terminating\n";
        return false;
    }
    catch(...)
    {
        std::cerr<<"ERROR: An unknown exception occured. Terminating\n";
        return false;
    }

    return true;
}

int MainApp::OnExit()
    throw()
{
    // we do not delete main_frame; wxWidgets magic does it for us
    return wxApp::OnExit();
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Event Tables //////////////////////////////
////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
    //EVT_TEXT_ENTER(MainFrame::ID_INPUT_BOX, MainFrame::OnEnter)
    EVT_COMMAND(wxID_ANY, nuke_msEVT_PRINT_MESSAGE, MainFrame::OnPrintMessage)
END_EVENT_TABLE()


/*
int main(int argc, char** argv)
{
*/
IMPLEMENT_APP(MainApp) // <-- main function is somewhere deep inside this macro
/*
}
*/





