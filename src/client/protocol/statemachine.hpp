// statemachine.hpp

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


#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/statechart/asynchronous_state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/mpl/list.hpp>
#include <boost/ref.hpp>

#include "msglayer.hpp"
#include "control/notifications.hpp"



namespace nuke_ms
{
namespace protocol
{

// Event declarations

/** Event representing a Connection Request.
* @ingroup proto_machine
*/
struct EvtConnectRequest : public boost::statechart::event<EvtConnectRequest>
{
    /** Where to connect to. */
    std::string host;
    std::string service;

    /** Constructor.
    * @param _where Where to connect to. */
    EvtConnectRequest(const std::string& _host, const std::string& _service)
        : host (_host), service(_service)
    {}
};

/** Event informing about the outcome of an attempt to connect to a remote site.
* @ingroup proto_machine
*
*/
struct EvtConnectReport : public boost::statechart::event<EvtConnectReport>
{
    bool success; /**< true on success, false on failure */
    byte_traits::string message; /**< Message commenting the outcome. */

    /** Constructor. Initializes members.
    * @param _success true on success, false on failure
    * @param _message Message commenting the outcome.
    */
    EvtConnectReport(bool _success, const byte_traits::string& _message)
        : success(_success), message (_message)
    {}
};

/** Event representing a Disconnection Request.
* @ingroup proto_machine
*/
struct EvtDisconnectRequest :
    public boost::statechart::event<EvtDisconnectRequest>
{};


/** Event representing a Disconnect.
* @ingroup proto_machine
*/
struct EvtDisconnected :
    public boost::statechart::event<EvtDisconnected>
{
    /** The text of the message. */
    byte_traits::string msg;

    /** Constructor.
    * @param _msg The text of the message.
    */
    EvtDisconnected(const byte_traits::string& _msg)
        : msg (_msg)
    {}
};

/** Event representing a sent message
* @ingroup proto_machine
*/
struct EvtSendMsg : public boost::statechart::event<EvtSendMsg>
{
    /** The data of the message */
    BasicMessageLayer::ptr_type data;

    /** Constructor.
    * @param _data The text of the message.
    */
    EvtSendMsg(BasicMessageLayer::ptr_type _data)
        : data (_data)
    {}
};

/** Event representing a received message */
struct EvtRcvdMessage :
    public boost::statechart::event<EvtRcvdMessage>
{
    /** The data of the message. */
    SegmentationLayer data;

    /** Constructor.
    * @param _msg The text of the message.
    */
    EvtRcvdMessage(const SegmentationLayer& _data)
        : data (_data)
    {}
};

// Forward declaration of the Initial State
struct StateWaiting;



/** The Protoc State Machine.
* @ingroup proto_machine
* This class represents the Overall State of the Protocol.
* Events can be dispatched to this machine, and the according actions will be
* performed.
*
* Currently, this class is an "asynchronous_state_machine", which has it's own
* thread. Refer to the documention of Boost.Statechart for details on how to
* create and use this class, and how to dispatch events.
*/
struct ProtocolMachine :
    public boost::statechart::asynchronous_state_machine<
        ProtocolMachine,
        StateWaiting
    >
{
    enum {thread_timeout = 3000u};

    /** Callback that will be used to inform the application */
    nuke_ms::control::notif_callback_t notification_callback;

    /** I/O Service Object */
    boost::asio::io_service io_service;

    /** Socket used for the connection */
    boost::asio::ip::tcp::socket socket;


    /** A thread object for all asynchronouy I/O operations. It will start in
    not-a-thread state. */
    boost::thread io_thread;


    /** Constructor. To be used only by Boost.Statechart classes.
    * Passing _io_service by pointer, because passing references to
    * create_processor does not work.
    */
    ProtocolMachine(my_context ctx,
                    nuke_ms::control::notif_callback_t _notification_callback);


    /** Destructor. Stops all I/O operations and threads as cleanly as possible.
    */
    ~ProtocolMachine();

    /** Starts the I/O service object and thread to process I/O operations.
    * @pre io_service object must have some work to do before calling this
    * function.
    *
    * @throws std::exception if an error creating the thread occurs.
    */
    void startIOOperations() throw(std::exception);

    /** Stops I/O Operations.
    * This function stops the IO- Service Object and joins the thread if
    * they are running.
    */
    void stopIOOperations() throw();
};



/** State indicating that the Protocol is Waiting to be connected.
* @ingroup proto_machine
*
* Reacting to:
* EvtConnectRequest
* EvtSendMsg
*/
struct StateWaiting :
    public boost::statechart::state<StateWaiting, ProtocolMachine>
{

    /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvtConnectRequest >,
        boost::statechart::custom_reaction<EvtSendMsg>
    > reactions;

    /** Constructor. To be used only by Boost.Statechart classes. */
    StateWaiting(my_context ctx);

    boost::statechart::result react(const EvtConnectRequest &);
    boost::statechart::result react(const EvtSendMsg &);

};


/** State indicating that the Connection is being established.
* @ingroup proto_machine
*
* Reacting to:
* EvtConnectReport
*/
struct StateNegotiating :
    public boost::statechart::state<StateNegotiating, ProtocolMachine>
{
     /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvtConnectReport >,
        boost::statechart::custom_reaction< EvtDisconnectRequest >,
        boost::statechart::custom_reaction< EvtSendMsg >
    > reactions;

    /** Constructor. To be used only by Boost.Statechart classes. */
    StateNegotiating(my_context ctx);

    static void tiktakHandler(
        const boost::system::error_code& error,
        boost::shared_ptr<boost::asio::deadline_timer> timer,
        outermost_context_type& _outermost_context
    );

    static void resolveHandler(
        const boost::system::error_code& error,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
        outermost_context_type& _outermost_context,
        boost::shared_ptr<boost::asio::ip::tcp::resolver> /* resolver */,
        boost::shared_ptr<boost::asio::ip::tcp::resolver::query> /* query */
    );


    static void connectHandler(
        const boost::system::error_code& error,
        outermost_context_type& _outermost_context
    );


    boost::statechart::result react(const EvtConnectReport& evt);
    boost::statechart::result react(const EvtDisconnectRequest&);
    boost::statechart::result react(const EvtSendMsg& evt);
};


struct StateConnected :
    public boost::statechart::state<StateConnected, ProtocolMachine>
{
    /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<EvtDisconnectRequest>,
        boost::statechart::custom_reaction<EvtSendMsg>,
        boost::statechart::custom_reaction<EvtDisconnected>,
        boost::statechart::custom_reaction<EvtRcvdMessage>
    > reactions;

    /** Constructor. To be used only by Boost.Statechart classes. */
    StateConnected(my_context ctx);

    boost::statechart::result react(const EvtDisconnectRequest&);
    boost::statechart::result react(const EvtSendMsg& evt);
    boost::statechart::result react(const EvtDisconnected& evt);
    boost::statechart::result react(const EvtRcvdMessage& evt);

    static void writeHandler(
        const boost::system::error_code& error,
        std::size_t bytes_transferred,
        outermost_context_type& _outermost_context,
        SegmentationLayer::dataptr_type data
    );

    static void receiveSegmentationHeaderHandler(
        const boost::system::error_code& error,
        std::size_t bytes_transferred,
        outermost_context_type& _outermost_context,
        byte_traits::byte_t rcvbuf[SegmentationLayer::header_length]
    );

    static void receiveSegmentationBodyHandler(
        const boost::system::error_code& error,
        std::size_t bytes_transferred,
        outermost_context_type& _outermost_context,
        SegmentationLayer::dataptr_type rcvbuf
    );

};

// this function is declared in protocol.hpp and defined in protocol.hpp
extern void catchThread(boost::thread& thread, unsigned threadwait_ms) throw();


} // namespace protocol
} // namespace nuke_ms


#endif // ifndef STATEMACHINE_HPP
