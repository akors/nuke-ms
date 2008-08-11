// main.cpp

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

/** @file main.cpp
* @brief Application entry point
*
* This file contains the main() function and launches the application.
*
* @author Alexander Korsunsky
*
*/

#include <wx/app.h>

#include "control.hpp"
#include "gui.hpp"
#include "protocol.hpp"


using namespace nms;
using namespace gui;



/** Application entry point.
* @ingroup gui
* This class represents the main thread of execution.
*
* @todo Do semthing sensible when an exception is thrown.
*/
class MainApp : public wxApp
{

    /** Application control object*/
    control::AppControl<MainFrameWrapper, protocol::NMSProtocol>* app_control;

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
                app_control->getGui()->OnEnter(cmd_evt);
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
    // Create AppControl object, which creates its components
    try {
        app_control =
            new control::AppControl<MainFrameWrapper, protocol::NMSProtocol>;
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
    delete app_control;
    return wxApp::OnExit();
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Event Tables //////////////////////////////
////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
    //EVT_TEXT_ENTER(MainFrame::ID_INPUT_BOX, MainFrame::OnEnter)
END_EVENT_TABLE()


/*
int main(int argc, char** argv)
{
*/
IMPLEMENT_APP(MainApp) // <-- main function is somewhere deep inside this macro
/*
}
*/





