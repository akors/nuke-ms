// gui.cpp

/*
 *   nuke-ms - Nuclear Messaging System
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

#include "gui/gui.hpp"

#include "control/control.hpp"

namespace nuke_ms
{
namespace gui
{


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


void MainFrame::parseCommand(const byte_traits::string& str)
{
    // we are basically dealing only with Control commands, so it would be nice
    // if all the types were in scope
    using namespace control;

    // get ourself a tokenizer
    typedef boost::tokenizer<boost::char_separator<wchar_t>,
                             byte_traits::string::const_iterator, byte_traits::string >
        tokenizer;


    boost::char_separator<wchar_t> whitespace(L" \t\r\n");
    tokenizer tokens(str, whitespace);

    tokenizer::iterator tok_iter = tokens.begin();

    // bail out, if there were no tokens
    if ( tok_iter == tokens.end() )
        goto invalid_command;


    else if  ( !tok_iter->compare( L"/exit" ) )
    { signals.exitApp(); return; }

    else if  ( !tok_iter->compare( L"/disconnect" ) )
    { signals.disconnect(); return; }

    else if ( !tok_iter->compare( L"/print") )
    {
        if (++tok_iter == tokens.end() )
            goto invalid_command;

        printMessage(L"*  " + *tok_iter);
        return;
    }

    else if ( !tok_iter->compare( L"/connect") )
    {
        if (++tok_iter == tokens.end() )
            goto invalid_command;

        ServerLocation::ptr_t where(new ServerLocation);
        where->where = *tok_iter;
        signals.connectTo(where);
        return;
    }

invalid_command:
    //  if we reached this line the command is invalid
    printMessage(L"*  Invalid command syntax!");
}


void MainFrame::printMessage(const byte_traits::string& str)
    throw (std::runtime_error)
{
    // wxEvtHandler::AddPendingEvent is explicitly thread-safe, so we don't
    // need any mutexes and locking here.

    // create an event to be sent to the event processor
    wxCommandEvent event( nuke_msEVT_PRINT_MESSAGE, this->GetId() );

    // set the event object that shall receive the event
    event.SetEventObject( this );

    // create the wxString from byte_traits::string, and perform conversion
    wxString msg(str.c_str(), wxConvLocal);

    // attach the string to the event object
    event.SetString(msg);

    // send the event to the event processor
    GetEventHandler()->AddPendingEvent( event );
}


////////////////////////////// MainFrame Public ///////////////////////////////


MainFrame::MainFrame() throw()
    : wxFrame(NULL, -1, wxT("killer app"), wxDefaultPosition, wxSize(600, 500))
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
    throw()
{
    Close(true);
}



void MainFrame::OnEnter(wxCommandEvent& event)
    throw()
{
    // create reference to a byte_traits::string
    const byte_traits::string& input_string =
        wxString2wstring(text_input_box->GetValue() );

    // clear input box
    text_input_box->Clear();

    // nothing to do if the user entered nothing
    if ( input_string.empty() )
        return;

    // print everything the user typed to the screen
    printMessage(L"<< " + input_string);


    // check if this is a command
    if ( MainFrame::isCommand(input_string) )
    {
        MainFrame::parseCommand(input_string);
    }
    else // if it's not a command we try to send it
    {
        control::Message::ptr_t msg(new control::Message);
        msg->str = input_string;

        signals.sendMessage(msg);
    }

}

void MainFrame::OnPrintMessage(wxCommandEvent& event) throw()
{
    text_display_box->AppendText( event.GetString() + wxT('\n') );
}


// define the new event type for printing messages
DEFINE_EVENT_TYPE( nuke_msEVT_PRINT_MESSAGE )

} // namespace gui
} // namespace nuke_ms
