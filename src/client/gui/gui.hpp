// gui.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2010  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** @file gui.hpp
* @brief Graphical user interface
*
* This file contains the Graphical User Interface Layout and Logic. Commands
* that come from the user are sent to the Application Control entitiy.
*
* @author Alexander Korsunsky
*
*/

/** @defgroup gui Graphical User Interface */

#ifndef MAIN_HPP_INCLUDED
#define MAIN_HPP_INCLUDED

#include <boost/thread/mutex.hpp>
#include <boost/signals2/signal.hpp>



#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>

#include "clientnode/protocol.hpp"
#include "clientnode/sigtypes.hpp"

namespace nuke_ms
{
namespace gui
{

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




// Forward Declaration for friend declaration
class MainFrameWrapper;


/** Main Window.
* @ingroup gui
* This class represents the main window. Construct it and a window will appear.
*
*/
class MainFrame : public wxFrame
{

    /** Enum for wxWidgets window identifiers. */
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

    /** The mutex to ensure synchronized access to the display resource*/
    boost::mutex print_mutex;

    /** Network Protocol object */
    protocol::NukeMSProtocol protocol;



    /** creates and initializes menu bars*/
    void createMenuBar();

    /** creates and initializes text boxes */
    void createTextBoxes();


    /** Print a message to the Output text box.
    * Locks the printer mutex.
    *
    * @param str The string you want to print
    * @throws std::runtime_error if a ressource could not be allocated.
    * e.g. a threading resource.
    */
    void printMessage(const wxString& str);


    /** Check if a string is a command
    *
    * @param str The string you want to be checked
    * @todo Add proper checking here
    */
    inline static bool isCommand(const wxString& str)
    {
        return ((str.size() != 0) && (str[0] == wxT('/')));
    }


    /** Parse a string and interpret it as command.
    * You might want to check if this seems like a Command before calling this
    * function.
    *
    * @param str The string you want to be interpreted as command
    * @todo add proper parser here
    */
    void parseCommand(const wxString& str);

public:

    /** Constructor.
    *
    * Initialize base class, set scales,
    * create menu bar and text boxes, connect protocol signals and slots.
    */
    MainFrame();



    void slotReceiveMessage(NearUserMessage::const_ptr_t msg);
    void slotConnectionStatusReport(
        protocol::ConnectionStatusReport::const_ptr_t rprt);
    void slotSendReport(protocol::SendReport::const_ptr_t rprt);


    /** Called if the user wants to quit. */
    void OnQuit(wxCommandEvent& event);

    /** Called when the user hits the return key without
    * holding down the shift or ctrl. key
    */
    void OnEnter(wxCommandEvent& event);

    /** Called internally, if there is a message to be printed */
    void OnPrintMessage(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};




// declare the new event type for printing messages
DECLARE_LOCAL_EVENT_TYPE( nuke_msEVT_PRINT_MESSAGE, -1 )




/** create a byte_traits::msg_string object from a wxString.
*
* @param wxstr The string you want to convert
* @param str the output string
* @warning Probably not portable
*/
inline void wxString2str(byte_traits::msg_string& str, const wxString& wxstr)
{
    str.assign(wxstr.ToUTF8().data());
}


/** create a wxString object from an byte_traits::msg_string.
*
* @param str The string you want to convert
* @return the output string
* @warning Probably not portable
*/
inline void str2wxString(wxString& wxstr, const byte_traits::msg_string& str)
{
    wxstr = wxString::FromUTF8(str.c_str());
}


} // namespace gui
} // namespace nuke_ms

#endif // ifndef MAIN_HPP_INCLUDED


