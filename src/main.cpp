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
	wxBoxSizer* vertsizer = new wxBoxSizer(wxVERTICAL);


	text_display_box = new wxTextCtrl(this, wxID_ANY, wxT(""),
				wxDefaultPosition, scales.displaybox_size, 
				wxTE_READONLY | wxTE_MULTILINE);

	text_input_box = new wxTextCtrl(this, ID_INPUT_BOX, wxT(""),
				wxDefaultPosition, scales.inputbox_size, 
				wxTE_PROCESS_ENTER | wxTE_MULTILINE);

	vertsizer->Add(
			text_display_box,
			scales.displaybox_proportion, 
			wxALL  | wxEXPAND | wxALIGN_CENTER_HORIZONTAL,
			scales.border_width
			  );

	vertsizer->Add(
			text_input_box,
			scales.inputbox_proportion, 
			wxALL | wxEXPAND | wxALIGN_CENTER_HORIZONTAL,
			scales.border_width
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
	if  ( !str.compare( L"/exit" ) )
		return boost::shared_ptr<ControlCommand>
			(new ControlCommand(ControlCommand::ID_EXIT));
	
	if ( !str.compare( L"/print") )
		return boost::shared_ptr<ControlCommand> 
			(new Command_PrintMessage(L"Hallo!"));
	
	return boost::shared_ptr<ControlCommand> 
			(new ControlCommand(ControlCommand::ID_INVALID));
}


void MainFrame::printMessage(const std::wstring& str)
{
    text_display_box->AppendText( str.c_str() );
}


void MainFrame::printMessage(const wxString& str)
{
    text_display_box->AppendText( str );
}




////////////////////////////// MainFrame Public ///////////////////////////////


MainFrame::MainFrame()
	: wxFrame(NULL, -1, wxT("killer app"), wxDefaultPosition, wxSize(600, 500))
{
	
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
	
    
    // check if this is a command
    if ( MainFrame::isCommand(input_string) )
    {
		// create a shared pointer from the output
		boost::shared_ptr<ControlCommand> cmd =
		    MainFrame::parseCommand(input_string) ;
		
		if ( cmd->id == ControlCommand::ID_INVALID )
		{
			app_control->printMessage(L"Invalid command!\n");
		}
		
		if ( cmd->id == ControlCommand::ID_EXIT )
		{
			app_control->close();
			return;
		}
		
		if (cmd->id == ControlCommand::ID_PRINT_MSG)
		{
			const Command_PrintMessage& cmd_msg =
				dynamic_cast<const Command_PrintMessage&> (*cmd);
			
			app_control->printMessage(cmd_msg.msg);
		}
    }
    else
    {
		// print only if the string is not empty
		if ( !input_string.empty() )
			app_control->printMessage(input_string +L'\n');
    }

	text_input_box->Clear();
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
            mainFrame->OnEnter(cmd_evt);
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
	mainFrame = new MainFrame();
	protocol = new NMSProtocol();
    app_control = new AppControl(mainFrame, protocol);
    
	return true;
}

int MainApp::OnExit()
{
    delete app_control;
    delete protocol;
    
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






