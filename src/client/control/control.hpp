// control.hpp

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


/** @file control.hpp
* @brief Application Control and Management
*
* This file contains the Application management. Commands and notifications that
* come from the components are processed and the appropriate actions are made.
*
* @author Alexander Korsunsky
*/



/** @defgroup app_ctrl Application Control */




#ifndef CONTROL_HPP_INCLUDED
#define CONTROL_HPP_INCLUDED

#include <iostream>

#include <stdexcept>

#include <boost/bind.hpp>

#include "bytes.hpp"

namespace nuke_ms
{
namespace control
{


/** Application Control Class.
* @ingroup app_ctrl
* This class controls the whole Application. It provides Callback functions
* that can be used by the components to report events. These callback functions
* determine the actions to be taken.
*
* @tparam GuiT Class of the GUI object.
* @tparam ProtocolT Class of the Connection Protocol object.
*
* @note Copy construction and assignment are not allowed.
*/
template <typename GuiT, typename ProtocolT>
class AppControl
{
    GuiT gui; /**< The GUI object */
    ProtocolT protocol; /**< The Communication protocol object */

private:

    AppControl& operator= (const AppControl&); /**<  not assignable */
    AppControl(const AppControl&); /**<   not copyable */


    /** Shut down the application. */
    void close() throw() { gui.close(); }

public:
    /** Constructor.
    * Registers the handleCommand callback with the gui object.
    *
    * Throws the same exceptions the constructors of the template arguments throw.
    */
    AppControl()
    {
        // this one's for us
        gui.connectExitApp(boost::bind(&AppControl::close, this));

        // thread gui signals to protocol slots
        gui.connectConnectTo(boost::bind(&ProtocolT::connect_to,&protocol,_1));
        gui.connectSendMessage(boost::bind(&ProtocolT::send, &protocol, _1));
        gui.connectDisconnect(boost::bind(&ProtocolT::disconnect,&protocol));

        // thread protocol signals to gui slots
        protocol.connectRcvMessage(
            boost::bind(&GuiT::slotReceiveMessage,&gui,_1));
        protocol.connectConnectionStatusReport(
            boost::bind(&GuiT::slotConnectionStatusReport,&gui,_1));
        protocol.connectSendReport(boost::bind(&GuiT::slotSendReport,&gui,_1));
    }


    /** Return a pointer to the GUI.
    * The main Application might need to access the GUI directly. This is
    * possible by calling this function.
    */
    GuiT* getGui() throw() { return &gui; }
};


} // namespace control

} // namespace nuke_ms



#endif // ifndef CONTROL_HPP_INCLUDED

