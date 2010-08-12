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

#include "control/sigtypes.hpp"

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


    /**
    */
    struct Signals
    {
        control::SignalConnectTo connectTo;
        control::SignalSendMessage sendMessage;
        control::SignalConnectionStatusQuery connectionStatusQuery;
        control::SignalDisconnect disconnect;
        control::SignalExitApp exitApp;
    } signals;



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
    void printMessage(const byte_traits::string& str)
        throw(std::runtime_error);



    void slotReceiveMessage(control::Message::const_ptr_t msg) throw();
    void slotConnectionStatusReport(
        control::ConnectionStatusReport::const_ptr_t rprt) throw();
    void slotSendReport(control::SendReport::const_ptr_t rprt) throw();

    /** Check if a string is a command
    *
    * @param str The string you want to be checked
    * @todo Add proper checking here
    */
    inline static bool isCommand(const byte_traits::string& str)
        throw()
    {
        return ((str.size() != 0) && (str[0] == L'/'));
    }


    /** Parse a string and interpret it as command.
    * You might want to check if this seems like a Command before calling this
    * function.
    *
    * @param str The string you want to be interpreted as command
    * @todo add proper parser here
    */
    void parseCommand(const byte_traits::string& str);

public:

    friend class MainFrameWrapper;

    /** Constructor.
    *
    * Initialize base class, set scales,
    * create menu bar and text boxes.
    *
    * @param _commandCallback A function object that will be called when the
    * user issues a command
    */
    MainFrame()  throw();


    boost::signals2::connection
    connectConnectTo(const control::SignalConnectTo::slot_type& slot)
    { return signals.connectTo.connect(slot); }

    boost::signals2::connection
    connectSendMessage(const control::SignalSendMessage::slot_type& slot)
    { return signals.sendMessage.connect(slot); }

    boost::signals2::connection
    connectConnectionStatusQuery(
        const control::SignalConnectionStatusQuery::slot_type& slot)
    { return signals.connectionStatusQuery.connect(slot); }

    boost::signals2::connection
    connectDisconnect(const control::SignalDisconnect::slot_type& slot)
    { return signals.disconnect.connect(slot); }

    boost::signals2::connection
    connectExitApp(const control::SignalExitApp::slot_type& slot)
    { return signals.exitApp.connect(slot); }

    /** Called if the user wants to quit. */
    void OnQuit(wxCommandEvent& event) throw();

    /** Called when the user hits the return key without
    * holding down the shift or ctrl. key
    */
    void OnEnter(wxCommandEvent& event) throw();

    /** Called internally, if there is a message to be printed */
    void OnPrintMessage(wxCommandEvent& event) throw();

    DECLARE_EVENT_TABLE()
};

/** Wrapper around the MainFrame class, to be used by the AppControl object.
* @ingroup gui
* This class is needed, because wxWindow objects (from which MainFrame is
* derived) can only be created by allocating it with new. However, AppControl
* stores the gui object locally.
* @see MainFrame
*/
class MainFrameWrapper
{
    /** The actual MainFrame class */
    MainFrame* main_frame;

    /** All outgoing signals */
    MainFrame::Signals *signals;
public:

    /** Constructor. Creates a new MainFrame object. */
    MainFrameWrapper() throw()
        : main_frame(NULL)
    {
        main_frame = new MainFrame;
        signals = &(main_frame->signals);
    }

    ~MainFrameWrapper()
        throw()
    {
        // MainFrame is derived from wxWindow. As such, simply deleting the
        // object would be incorrect. It has to delete itself.
    }


    boost::signals2::connection
    connectConnectTo(const control::SignalConnectTo::slot_type& slot)
    { return signals->connectTo.connect(slot); }

    boost::signals2::connection
    connectSendMessage(const control::SignalSendMessage::slot_type& slot)
    { return signals->sendMessage.connect(slot); }

    boost::signals2::connection
    connectConnectionStatusQuery(
        const control::SignalConnectionStatusQuery::slot_type& slot)
    { return signals->connectionStatusQuery.connect(slot); }

    boost::signals2::connection
    connectDisconnect(const control::SignalDisconnect::slot_type& slot)
    { return signals->disconnect.connect(slot); }

    boost::signals2::connection
    connectExitApp(const control::SignalExitApp::slot_type& slot)
    { return signals->exitApp.connect(slot); }


    void slotReceiveMessage(control::Message::const_ptr_t msg) throw()
    {
        main_frame->slotReceiveMessage(msg);
    }

    void slotConnectionStatusReport(
        control::ConnectionStatusReport::const_ptr_t rprt) throw()
    {
        main_frame->slotConnectionStatusReport(rprt);
    }

    void slotSendReport(control::SendReport::const_ptr_t rprt) throw()
    {
        main_frame->slotSendReport(rprt);
    }


    /** Print a message.
    * @see MainFrame::printMessage()
    * @throws std::runtime_error if a ressource could not be allocated.
    * e.g. a threading resource.
    */
    void printMessage(const byte_traits::string& str)
        throw (std::runtime_error)
    {
        main_frame->printMessage(str);
    }

    /** Closes the Main Frame. @see MainFrame::Close() */
    void close() throw()
    {
        main_frame->Close(false);
    }

    /** Called when the user hits the return key without
    * holding down the shift or ctrl. key
    */
    void OnEnter(wxCommandEvent& event) throw()
    {
        main_frame->OnEnter(event);
    }

};


// declare the new event type for printing messages
DECLARE_EVENT_TYPE( nuke_msEVT_PRINT_MESSAGE, -1 )




/** create a byte_traits::string object from a wxString.
*
* @param str The string you want to convert
* @return the output string
* @warning Probably not portable
*/
inline byte_traits::string wxString2wstring(const wxString& str)
    throw()
{
    return byte_traits::string( str.wc_str(wxConvLocal) );
}


/** create a wxString object from an byte_traits::string.
*
* @param str The string you want to convert
* @return the output string
* @warning Probably not portable
*/
inline wxString wstring2wxString(const byte_traits::string& str)
    throw()
{
    return wxString( str.c_str() );
}


} // namespace gui
} // namespace nuke_ms

#endif // ifndef MAIN_HPP_INCLUDED


