// gui.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2010  Alexander Korsunsky
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

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>

#include "client-wx/gui.hpp"


using namespace nuke_ms;
using namespace gui;


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


void MainFrame::parseCommand(const wxString& str)
{
    using namespace clientnode;

    // get ourself a tokenizer
    typedef boost::tokenizer<boost::char_separator<wxChar>, const wxChar*, wxString >
        tokenizer;


    boost::char_separator<wxChar> whitespace(wxT(" \t\r\n"));
    tokenizer tokens(str, whitespace);

    tokenizer::iterator tok_iter = tokens.begin();

    // bail out, if there were no tokens
    if ( tok_iter == tokens.end() )
        goto invalid_command;


    else if  ( !tok_iter->compare(wxT("/exit")) )
    { Close(); return; }

    else if  ( !tok_iter->compare(wxT("/disconnect")) )
    { clientnode.disconnect(); return; }

    else if ( !tok_iter->compare(wxT("/print")) )
    {
        if (++tok_iter == tokens.end() )
            goto invalid_command;

        printMessage(wxT("*  " + *tok_iter));
        return;
    }

    else if ( !tok_iter->compare(wxT("/connect")) )
    {
        if (++tok_iter == tokens.end() )
            goto invalid_command;

        byte_traits::native_string wherestr(tok_iter->begin(), tok_iter->end());

        ServerLocation::ptr_t where(new ServerLocation);
        where->where = wherestr;
        clientnode.connectTo(where);
        return;
    }

invalid_command:
    //  if we reached this line the command is invalid
    printMessage(L"*  Invalid command syntax!");
}



void MainFrame::slotReceiveMessage(NearUserMessage::const_ptr_t msg)
{
    byte_traits::msg_string s(msg->getStringwrap());
    printMessage(wxT(">> ") + wxString::FromUTF8(s.c_str()));
}

void MainFrame::slotConnectionStatusReport(
    clientnode::ConnectionStatusReport::const_ptr_t rprt)
{
    using namespace clientnode;

    switch(rprt->newstate)
    {
    case ConnectionStatusReport::CNST_DISCONNECTED:
        if (rprt->statechange_reason == ConnectionStatusReport::STCHR_NO_REASON
            || rprt->statechange_reason ==
                ConnectionStatusReport::STCHR_USER_REQUESTED)
        {
            printMessage(wxT("*  Connection state: disconnected."));
        }
        else
        {
            wxString rprtmsg;
            str2wxString(rprtmsg,rprt->msg);

            printMessage(
                wxT("*  New connection state: disconnected; ")+rprtmsg);
        }
        break;
    case ConnectionStatusReport::CNST_CONNECTING:
        if(!rprt->statechange_reason) // we only care if user wanted to know
            printMessage(wxT("*  Connection state: Connecting."));
        break;
    case ConnectionStatusReport::CNST_CONNECTED:
        if (!rprt->statechange_reason)
            printMessage(wxT("*  Connection state: connected."));
        else
            printMessage(wxT("*  New connection state: connected."));
        break;
    default:
        printMessage(wxT("*  Connection state: unknown."));
    }
}


void MainFrame::slotSendReport(clientnode::SendReport::const_ptr_t rprt)
{
    if (!rprt->send_state)
    {
        wxString rprtmsg;
        str2wxString(rprtmsg,rprt->reason_str);

        printMessage(wxT("*  Failed to send message: ") + rprtmsg);
    }
}


void MainFrame::printMessage(const wxString& str)
{
    // wxEvtHandler::AddPendingEvent is explicitly thread-safe, so we don't
    // need any mutexes and locking here.

    // create an event to be sent to the event processor
    wxCommandEvent event( nuke_msEVT_PRINT_MESSAGE, this->GetId() );

    // set the event object that shall receive the event
    event.SetEventObject( this );

    // attach the string to the event object
    event.SetString(str);

    // send the event to the event processor
    GetEventHandler()->AddPendingEvent( event );
}


////////////////////////////// MainFrame Public ///////////////////////////////


MainFrame::MainFrame()
    : wxFrame(NULL, -1, wxT("killer app"), wxDefaultPosition, wxSize(600, 500))
{
	// thread clientnode signals to gui slots
	clientnode.connectRcvMessage(
		boost::bind(&MainFrame::slotReceiveMessage, this, _1));
	clientnode.connectConnectionStatusReport(
		boost::bind(&MainFrame::slotConnectionStatusReport, this, _1));
	clientnode.connectSendReport(
		boost::bind(&MainFrame::slotSendReport, this, _1));

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
    wxString input_string(text_input_box->GetValue());

    // clear input box
    text_input_box->Clear();

    // nothing to do if the user entered nothing
    if ( !input_string )
        return;

    // print everything the user typed to the screen
    printMessage(wxT("<< ") + input_string);


    // check if this is a command
    if ( MainFrame::isCommand(input_string) )
    {
        MainFrame::parseCommand(input_string);
    }
    else // if it's not a command we try to send it
    {
        byte_traits::msg_string msg;
        wxString2str(msg,input_string);

        clientnode.sendUserMessage(msg);
    }

}

void MainFrame::OnPrintMessage(wxCommandEvent& event)
{
    text_display_box->AppendText( event.GetString() + wxT('\n') );
}

namespace nuke_ms { namespace gui {
	// define the new event type for printing messages
	DEFINE_LOCAL_EVENT_TYPE( nuke_msEVT_PRINT_MESSAGE )
}}

