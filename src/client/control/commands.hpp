// commands.hpp

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

/** @file commands.hpp
* @brief Command types.
*
* This file contains Types that are used to describe Commands that are sent
* from the GUI to the Application Control entity.
*
* @author Alexander Korsunsky
*/

/** @defgroup  ctrl_cmd Control Commands
* @ingroup app_ctrl */


#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>

namespace nuke_ms
{
namespace control
{




/** A command to the Application Control Class.
* @ingroup ctrl_cmd
* Send this command to AppControl.
*/
struct ControlCommand
{

    /** The Command ID Type
    */
    enum command_id_t {
        ID_INVALID = 0, /**< Invalid Command */
        ID_EXIT, /**< user asked to exit */
        ID_PRINT_MSG, /**< print a message */
        ID_SEND_MSG, /**< Send the message to the remote site */
        ID_CONNECT_TO, /**< connect to a remote site */
        ID_DISCONNECT, /**< disconnect from a remote site */

        /** Number of command identifiers.
        * As the first command (ID_INVALID) starts from 0,
        * this element specifies how many commands there are.
        */
        ID_NUM_COMMANDS
    };

    const command_id_t id; /**< The command ID*/


    /** Constructor. */
    ControlCommand(const command_id_t& _id) throw()
        : id(_id)
    {}

    virtual ~ControlCommand()
    {}
};

/** This template is for all commands that are used to transfer one string as
* payload.
* @ingroup ctrl_cmd
*/
template <ControlCommand::command_id_t CMD_ID>
struct MessageCommand : public ControlCommand
{
    /** The message you want to transport */
    const byte_traits::string msg;

    /** Constructor.
    * @param str The message you want to transport
    */
    MessageCommand(const byte_traits::string& str)  throw()
        : ControlCommand(CMD_ID), msg(str)
    { }
};


} // namespace control

} // namespace nuke_ms


#endif // ifndef COMMANDS_HPP

