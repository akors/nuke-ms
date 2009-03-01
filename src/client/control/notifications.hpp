// notifications.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2009  Alexander Korsunsky
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

/** @file notifications.hpp
* @brief Notification types.
*
* This file contains Types that are used to describe Notifications that are sent
* from the Protocol to the Application Control entity.
*
* @author Alexander Korsunsky
*/

/** @defgroup  notif Notification Messages
* @ingroup app_ctrl */


#ifndef NOTIFICATIONS_HPP
#define NOTIFICATIONS_HPP

#include <string>

#include <boost/function.hpp>

namespace nuke_ms
{
namespace control
{



/** A notification from the Communication protocol regarding an event
* @ingroup notif
*/
struct ProtocolNotification
{
    /** The type of operation that is reported about.
    */
    enum notification_id_t {
        ID_CONNECT_REPORT, /**< Report regarding a connection attempt */
        ID_SEND_REPORT, /**< Report regarding a sent message */
        ID_RECEIVED_MSG, /**< Message received */
        ID_DISCONNECTED /**< Disconnected */
    };

    /** What kind of Notification */
    const notification_id_t id;

    /** Construct a Report denoting failure */
    ProtocolNotification(notification_id_t _id)  throw()
        : id(_id)
    {}

    virtual ~ProtocolNotification()
    {}
};

template <ProtocolNotification::notification_id_t NotificationId>
struct ProtocolNotificationMessage : public ProtocolNotification
{
    /** The message that was received */
    const std::wstring msg;

    /** Constructor.
    * @param _msg The message that was received
    */
    ProtocolNotificationMessage(const std::wstring& _msg)  throw()
        :  ProtocolNotification(NotificationId), msg(_msg)
    {}
};


/** Notification regarding a received message
* @ingroup notif*/
typedef ProtocolNotificationMessage<ProtocolNotification::ID_RECEIVED_MSG>
    ReceivedMsgNotification;


/** Notification regarding a disconnected socket
* @ingroup notif*/
typedef ProtocolNotificationMessage<ProtocolNotification::ID_DISCONNECTED>
    DisconnectedNotification;


/** This class represents a positive or negative reply to a requested operation.
* @ingroup notif
*/
struct RequestReport
{
    const bool successful; /**< Good news or bad news? */
    const std::wstring failure_reason; /**< What went wrong? */

    /** Construct a positive report */
    RequestReport()  throw()
        : successful(true)
    {}

    /** Construct a negative report */
    RequestReport(const std::wstring& reason)  throw()
        : successful(false), failure_reason(reason)
    {}

    virtual ~RequestReport()
    {}
};

/** A generic notification that involves reporting about Success or failure */
template <ProtocolNotification::notification_id_t NOTIF_ID>
struct ReportNotification : public ProtocolNotification, public RequestReport
{
    /** Generate a successful report */
    ReportNotification()  throw()
        : ProtocolNotification(NOTIF_ID), RequestReport()
    {}

    /** Generate a negative report */
    ReportNotification(const std::wstring& reason)  throw()
        : ProtocolNotification(NOTIF_ID), RequestReport(reason)
    {}
};

#if 1
/** Report about the status of a sent Message.*/
struct SendReport :
    public ReportNotification<ProtocolNotification::ID_SEND_REPORT>
{
    /** The message that was supposed to be sent. */
    std::wstring message;

    /** Generate a successful report.
     * @param _message The message that was supposed to be sent.
     */
    SendReport(std::wstring _message)  throw()
        : ReportNotification<ProtocolNotification::ID_SEND_REPORT>(),
            message(_message)
    {}

    /** Generate a negative report
     * @param _message The message that was supposed to be sent.
     * @param _failure_reason The reason why the sending failed.
     */
    SendReport(std::wstring _message, std::wstring _failure_reason)  throw()
        : ReportNotification<ProtocolNotification::ID_SEND_REPORT>(
            _failure_reason),
        message(_message)
    {}
};
#endif

/** A typedef for the notification callback */
typedef boost::function1 <void, const control::ProtocolNotification&>
    notif_callback_t;


} // namespace control

} // namespace nuke_ms

#endif // ifndef NOTIFICATIONS_HPP
