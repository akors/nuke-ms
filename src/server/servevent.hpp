// servevent.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2009  Alexander Korsunsky
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

#ifndef SERVEVENT_HPP
#define SERVEVENT_HPP

#include <boost/function.hpp>
#include "msglayer.hpp"

namespace nuke_ms
{

/** Basic event sent to the server.
* This event will be passed to an event handler function when something
* interesting happens to the Socket, such as a received message or a
* disconnection. The handler function then takes the appropriate actions.
* To identify what kind of event has happened, an enum variable has to be set
* by the constructor.
* To identify the sender of the event, an id variable has to be set by the
* constructor.
*/
struct BasicServerEvent
{
    /** Type that identifies the connection to which this event happened. */
    typedef int connection_id_t;

    /** Enum for the kind of the Event that happened. */
    enum event_kind_t {
        ID_MSG_RECEIVED,
        ID_CONNECTION_ERROR,
        ID_CAN_DELETE
    };

    event_kind_t event_kind; /**< What kind of event has happened */
    connection_id_t connection_id; /**< To whom the event happened */

    /** Constructor.
    * Sets the event kind and connection id variables.
    */
    BasicServerEvent(
        event_kind_t _event_kind,
        connection_id_t _connection_id
    )
        : event_kind(_event_kind), connection_id(_connection_id)
    {}

    virtual ~BasicServerEvent() {}
};

/**  Server Event with one parameter.
* This class template is to be used for Server events, which contain a single
* parameter.
*/
template <BasicServerEvent::event_kind_t EventKind, typename ParmType>
struct ServerEvent1Parm : public BasicServerEvent
{
    /** The parameter that is passed with the event */
    ParmType parm;

    /** Constructor.
    * @param _parm The parameter that is passed with the event
    */
    ServerEvent1Parm(
        BasicServerEvent::connection_id_t _connection_id,
        const ParmType& _parm
    )  throw()
        :  BasicServerEvent(EventKind, _connection_id), parm(_parm)
    {}

    virtual ~ServerEvent1Parm() {}
};

/** Typedef for received message. */
typedef ServerEvent1Parm<
    BasicServerEvent::ID_MSG_RECEIVED, SegmentationLayer::ptr_t>
    ReceivedMessageEvent;

/** Typedef for Disconnection events. */
typedef ServerEvent1Parm<
    BasicServerEvent::ID_CONNECTION_ERROR, byte_traits::native_string>
    ConnectionErrorEvent;



/** A typedef for the notification callback */
typedef boost::function1 <void, const BasicServerEvent&>
    event_callback_t;

} // nuke_ms

#endif // ifndef SERVEVENT_HPP