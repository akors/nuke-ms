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


#include "main.hpp"
#include <wx/aboutdlg.h>
#include <boost/tokenizer.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>



////////////////////////////////////////////////////////////////////////////////
////////////////////////////// MainFrame Methods //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////// MainFrame Private //////////////////////////////

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


boost::shared_ptr<ControlCommand> 
MainFrame::parseCommand(const std::wstring& str)
{
    // get ourself a tokenizer
    typedef boost::tokenizer<boost::char_separator<wchar_t>,
                             std::wstring::const_iterator, std::wstring >
        tokenizer;        
    boost::char_separator<wchar_t> whitespace(L" \t\r\n");
    tokenizer tokens(str, whitespace);
    
    tokenizer::iterator tok_iter = tokens.begin();

    // bail out, if there were no tokens
    if ( tok_iter == tokens.end() )
        goto invalid_command;


	if  ( !tok_iter->compare( L"/exit" ) )	
		return boost::shared_ptr<ControlCommand>
			(new ControlCommand(ControlCommand::ID_EXIT));
			
    if  ( !tok_iter->compare( L"/disconnect" ) )	
		return boost::shared_ptr<ControlCommand>
			(new ControlCommand(ControlCommand::ID_DISCONNECT));
			
	
	else if ( !tok_iter->compare( L"/print") )
	{
	    if (++tok_iter == tokens.end() )
            goto invalid_command;
	        
		return boost::shared_ptr<ControlCommand> 
			(new Command_PrintMessage(*tok_iter));
	}
			
	else if ( !tok_iter->compare( L"/connect") )	
	{	    
	    if (++tok_iter == tokens.end() )
            goto invalid_command;
            
		return boost::shared_ptr<ControlCommand> 
			(new Command_ConnectTo(*tok_iter));		
	}
	
	// if comparison failed, the command is invalid		
			
invalid_command:
	return boost::shared_ptr<ControlCommand> 
			(new ControlCommand(ControlCommand::ID_INVALID));
}


void MainFrame::printMessage(const std::wstring& str)
{
    text_display_box->AppendText( (str + L'\n').c_str() );
}


void MainFrame::printMessage(const wxString& str)
{
    text_display_box->AppendText( str + L'\n' );
}




////////////////////////////// MainFrame Public ///////////////////////////////


MainFrame::MainFrame
        (boost::function1<void, const ControlCommand&> _commandCallback)
	: wxFrame(NULL, -1, wxT("killer app"), wxDefaultPosition, wxSize(600, 500)),
	commandCallback(_commandCallback)
{
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
{
	Close(true);
}




void MainFrame::OnEnter(wxCommandEvent& event)
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
	commandCallback(Command_PrintMessage(input_string));		

	
    
    // check if this is a command
    if ( MainFrame::isCommand(input_string) )
    {
		// create a shared pointer from the output
		boost::shared_ptr<ControlCommand> cmd =
		    MainFrame::parseCommand(input_string);
		
        commandCallback(*cmd);
    }
    else // if it's not a command we try to send it
    {   
        commandCallback(Command_SendMessage(input_string));  
    }

}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////// MainApp Methods //////////////////////////////
////////////////////////////////////////////////////////////////////////////////


bool MainApp::ProcessEvent(wxEvent& event)
{
    // check for key down event
    if (event.GetEventType() == wxEVT_CHAR)
    {
        
        // only if control and shift are NOT down, call OnEnter
        if ( ((wxKeyEvent&) event).GetKeyCode() == WXK_RETURN &&
            !((wxKeyEvent&) event).ControlDown() &&
            !((wxKeyEvent&) event).ShiftDown() 
            )
        {
            wxCommandEvent cmd_evt(wxEVT_COMMAND_ENTER);
            //main_frame->OnEnter(cmd_evt);
            return true;
        }
    }    

    // dont stop processing the event in this function
    event.Skip();
    
    // call base class version of this function
    return wxEvtHandler::ProcessEvent(event);
}



bool MainApp::OnInit()
{    
    // create window, protocol and control objects, and tie them all together
    app_control = new AppControl<MainFrameWrapper, NMSProtocol>;
    
	return true;
}

int MainApp::OnExit()
{
    // main_frame deleted by wxWidgets
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






