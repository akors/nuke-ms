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

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>

#include <wx/aboutdlg.h>

#include "main.hpp"

using namespace nms;
using namespace gui;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// MainFrame Methods //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////// MainFrame Private //////////////////////////////

boost::function1<void, wxCommandEvent&> MainFrame::OnEnter_callback;

void MainFrame::createMenuBar()
{
    // create menu bar
    menu_bar = new wxMenuBar();

    // create menu
    menu_file = new wxMenu();
    menu_file->Append(wxID_EXIT, wxT("&Quit"), wxT("Exit Application"));

    // append menu to menu bar
    menu_bar->Append(menu_file, wxT("File"));

    // register menu bar to the main window
    this->SetMenuBar(menu_bar);
}


void MainFrame::createTextBoxes()
{
    // create vertical sizer
    wxBoxSizer* vertsizer = new wxBoxSizer(wxVERTICAL);

    // create text box for displaying text (read only)
    text_display_box = new wxTextCtrl(this, wxID_ANY, wxT(""),
                wxDefaultPosition, scales.displaybox_size, 
                wxTE_READONLY | wxTE_MULTILINE);

    // create text box for writing text
    text_input_box = new wxTextCtrl(this, ID_INPUT_BOX, wxT(""),
                wxDefaultPosition, scales.inputbox_size, 
                wxTE_PROCESS_ENTER | wxTE_MULTILINE);

    // add the upper textbox to the sizer
    vertsizer->Add(
            text_display_box, // the text box we want to add
            scales.displaybox_proportion,  // the relative size
            wxALL | // border everywhere
            wxEXPAND |  // item expands to fill space 
            wxALIGN_CENTER_HORIZONTAL, 
            scales.border_width // border size
              );

    // add the lower textbox to the sizer
    vertsizer->Add(
            text_input_box, // the text box we want to add
            scales.inputbox_proportion,  // the relative size
            wxALL | // border everywhere
            wxEXPAND |  // item expands to fill space 
            wxALIGN_CENTER_HORIZONTAL, 
            scales.border_width // border size
              );

    // assign sizer to window
    this->SetSizer(vertsizer);

    // set limitations on minimum size
    this->SetSizeHints(vertsizer->GetMinSize());
    
    // set focus on the input box
    text_input_box->SetFocus();
}


boost::shared_ptr<control::ControlCommand> 
MainFrame::parseCommand(const std::wstring& str)
{
    // we are basically dealing only with Control commands, so it would be nice
    // if all the types were in scope
    using namespace control;

    // get ourself a tokenizer
    typedef boost::tokenizer<boost::char_separator<wchar_t>,
                             std::wstring::const_iterator, std::wstring >
        tokenizer;        
        
try {
    boost::char_separator<wchar_t> whitespace(L" \t\r\n");
    tokenizer tokens(str, whitespace);
    
    tokenizer::iterator tok_iter = tokens.begin();

    // bail out, if there were no tokens
    if ( tok_iter == tokens.end() )
        throw false;


    if  ( !tok_iter->compare( L"/exit" ) )    
        return boost::shared_ptr<ControlCommand>
            (new ControlCommand(ControlCommand::ID_EXIT));
            
    if  ( !tok_iter->compare( L"/disconnect" ) )    
        return boost::shared_ptr<ControlCommand>
            (new ControlCommand(ControlCommand::ID_DISCONNECT));
            
    
    else if ( !tok_iter->compare( L"/print") )
    {
        if (++tok_iter == tokens.end() )
            throw false;
            
        return boost::shared_ptr<ControlCommand> 
            (new MessageCommand<ControlCommand::ID_PRINT_MSG>(*tok_iter));
    }
            
    else if ( !tok_iter->compare( L"/connect") )    
    {        
        if (++tok_iter == tokens.end() )
            throw false;
            
        return boost::shared_ptr<ControlCommand> 
            (new MessageCommand<ControlCommand::ID_CONNECT_TO>(*tok_iter));        
    }
        
            
}
catch(...) {} 
// if somebody threw something, or if we reached this line,
// the command is invalid

    return boost::shared_ptr<ControlCommand> 
            (new ControlCommand(ControlCommand::ID_INVALID));
}


void MainFrame::printMessage(const std::wstring& str)
    throw (std::runtime_error)
{
    // lock the printer mutex while printing
    boost::lock_guard<boost::mutex> printlock(print_mutex);
    
    text_display_box->AppendText( (str + L'\n').c_str() );
}


////////////////////////////// MainFrame Public ///////////////////////////////


MainFrame::MainFrame
        (boost::function1<void, const control::ControlCommand&> _commandCallback)
    throw()
        
    : wxFrame(NULL, -1, wxT("killer app"), wxDefaultPosition, wxSize(600, 500)),
    commandCallback(_commandCallback)
{
    // Opening a hole for MainApp to get to the OnEnter member function
    OnEnter_callback = boost::bind(&MainFrame::OnEnter, this, _1);

    // softcoding the window sizes
    scales.border_width = 10;
    scales.displaybox_size = wxSize(300, 50);
    scales.inputbox_size = wxSize(300, 50);
    scales.displaybox_proportion = 3;
    scales.inputbox_proportion = 1;
    
    createMenuBar();
    createTextBoxes();

    Show(true);
}



void MainFrame::OnQuit(wxCommandEvent& event)
    throw()
{
    Close(true);
}



void MainFrame::OnEnter(wxCommandEvent& event)
    throw()
{
    // create reference to a std::wstring
    const std::wstring& input_string = 
        wxString2wstring(text_input_box->GetValue() ); 
    
    // clear input box
    text_input_box->Clear();
    
    // nothing to do if the user entered nothing
    if ( input_string.empty() )
        return;
        
    // print everything the user typed to the screen
    commandCallback(
            control::MessageCommand<control::ControlCommand::ID_PRINT_MSG>
                (input_string)
        );
    
    
    // check if this is a command
    if ( MainFrame::isCommand(input_string) )
    {
        // create a shared pointer from the output
        boost::shared_ptr<control::ControlCommand> cmd =
            MainFrame::parseCommand(input_string);
        
        commandCallback(*cmd);
    }
    else // if it's not a command we try to send it
    {   
        commandCallback(control::MessageCommand
                            <control::ControlCommand::ID_SEND_MSG>
                            (input_string));  
    }

}


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

                MainFrame::OnEnter_callback(cmd_evt);

                
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



IMPLEMENT_APP(MainApp)






