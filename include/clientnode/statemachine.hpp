// statemachine.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2009, 2010, 2011  Alexander Korsunsky
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

/** @file clientnode/statemachine.hpp
* @brief Internal statemachine that represents the current ClientNode state
* @ingroup clientnode
*
* @author Alexander Korsunsky
*/

#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/asio.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/mpl/list.hpp>
#include <boost/ref.hpp>

#include "msglayer.hpp"
#include "clientnode/logstreams.hpp"
#include "clientnode/sigtypes.hpp"
#include "refcounter.hpp"

namespace nuke_ms
{


/** @defgroup proto_machine Communication Protocol State Machine
* @ingroup clientnode
* @{
*/

namespace clientnode
{

// Event declarations

/** Event representing a Connection Request.
* @ingroup proto_machine
*/
struct EvtConnectRequest : public boost::statechart::event<EvtConnectRequest>
{

    byte_traits::native_string host; /**< Where to connect to. */
    byte_traits::native_string service; /**< Which port to connect to. */

    /** Constructor.
    * @param _host Where to connect to.
	* @param _service Which port to connect to.
	*/
    EvtConnectRequest(const byte_traits::native_string& _host,
        const byte_traits::native_string& _service)
        : host {_host}, service{_service}
    {}
};

/** Event informing about the outcome of an attempt to connect to a remote site.
* @ingroup proto_machine
*
*/
struct EvtConnectReport : public boost::statechart::event<EvtConnectReport>
{
    bool success; /**< true on success, false on failure */
    byte_traits::native_string message; /**< Message commenting the outcome. */

    /** Constructor. Initializes members.
    * @param _success true on success, false on failure
    * @param _message Message commenting the outcome.
    */
    EvtConnectReport(bool _success, const byte_traits::native_string& _message)
        : success{_success}, message {_message}
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
    /** The reason of the disconnection event */
    byte_traits::native_string msg;

    /** Constructor.
    * @param _msg The text of the message.
    */
    EvtDisconnected(const byte_traits::native_string& _msg)
        : msg {_msg}
    {}
};

/** Event representing a sent message
* @ingroup proto_machine
*/
template <typename MessageType>
struct EvtSendMsg : public boost::statechart::event<EvtSendMsg<MessageType>>
{
    /** The data of the message */
    std::shared_ptr<MessageType> _data;

    /** Constructor.
    * @param _data The text of the message.
    */
    EvtSendMsg(MessageType&& data)
        : _data{std::make_shared<MessageType>(std::move(data))}
    {}

    EvtSendMsg(const EvtSendMsg&) = default;
};

/** Event representing a received message
* @ingroup proto_machine
* @ingroup netdata
*/
template <typename UpperLayer>
struct EvtRcvdMessage :
    public boost::statechart::event<EvtRcvdMessage<UpperLayer>>
{
    /** The data of the message. */
    std::shared_ptr<SegmentationLayer<UpperLayer>> _data;

    /** Constructor.
    * @param _data The data of the message.
    */
    EvtRcvdMessage(SegmentationLayer<UpperLayer>&& data)
        :_data{std::make_shared<SegmentationLayer<UpperLayer>>(std::move(data))}
    {}

    EvtRcvdMessage(const EvtRcvdMessage<UpperLayer>&) = default;
};


// Forward declaration of the Initial State
struct StateWaiting;


/** The Protoc State Machine.
* @ingroup proto_machine
* This class represents the Overall State of the clientnode.
* Events can be dispatched to this machine, and the according actions will be
* performed.
*
* Currently, this class is an "asynchronous_state_machine", which has it's own
* thread. Refer to the documention of Boost.Statechart for details on how to
* create and use this class, and how to dispatch events.
*/
class ClientnodeMachine :
    public boost::statechart::state_machine<
        ClientnodeMachine,
        StateWaiting
    >,
    public ReferenceCounter<ClientnodeMachine>

{
    std::shared_ptr<boost::asio::io_service> io_service;

    /** A thread object for all asynchronouy I/O operations. It will start in
    not-a-thread state. */
    boost::thread io_thread;

    /**
     * Condition that is true when all currently invoked handlers have returned
    */
    boost::condition_variable returned_condition;

    /** Callback that will be called when all handlers have returned */
    void on_returned()
    { returned_condition.notify_all(); }


public:
    enum {thread_timeout = 3000u};

    /** Callback signals that will be used to inform the application */
    ClientNodeSignals& signals;

    /** The Streams used for message output */
	LoggingStreams logstreams;

    /** Socket used for the connection */
    boost::asio::ip::tcp::socket socket;

    /** Resolver used for any resolve operations */
    boost::asio::ip::tcp::resolver resolver;

    /** A reference to the mutex that is needed to access this machine */
    boost::mutex& machine_mutex;


    /** Constructor.
    */
    ClientnodeMachine(ClientNodeSignals&  _signals,
		LoggingStreams logstreams_, boost::mutex& _machine_mutex);


    /** Destructor. Stops all I/O operations and threads as cleanly as possible.
    */
    ~ClientnodeMachine();

    /** Starts the I/O service object and thread to process I/O operations.
    * @pre io_service object must have some work to do before calling this
    * function.
    *
    * @throws std::exception if an error creating the thread occurs.
    */
    void startIOOperations();

    /** Stops I/O Operations.
    * This function stops the IO- Service Object and joins the thread if
    * they are running.
    */
    void stopIOOperations();

};



/** State indicating that the Protocol is Waiting to be connected.
* @ingroup proto_machine
*
* Reacting to:
* EvtConnectRequest
* EvtSendMsg
*/
struct StateWaiting :
    public boost::statechart::state<StateWaiting, ClientnodeMachine>
{

    /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<EvtConnectRequest>,
        boost::statechart::custom_reaction<EvtSendMsg<NearUserMessage>>
    > reactions;

    /** Constructor. To be used only by Boost.Statechart classes. */
    StateWaiting(my_context ctx);

    boost::statechart::result react(const EvtConnectRequest&);
    boost::statechart::result react(const EvtSendMsg<NearUserMessage>&);

};


/** State indicating that the Connection is being established.
* @ingroup proto_machine
*
* Reacting to:
* EvtConnectReport
*/
struct StateNegotiating :
    public boost::statechart::state<StateNegotiating, ClientnodeMachine>
{
     /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<EvtConnectReport>,
        boost::statechart::custom_reaction<EvtDisconnectRequest>,
        boost::statechart::custom_reaction<EvtSendMsg<NearUserMessage>>, // FIXME Use a template version of this function!
        boost::statechart::custom_reaction<EvtConnectRequest>
    > reactions;

    /** Constructor. To be used only by Boost.Statechart classes. */
    StateNegotiating(my_context ctx);

    static void resolveHandler(
        const boost::system::error_code& error,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
        ClientnodeMachine::CountedReference cm,
        std::shared_ptr<boost::asio::ip::tcp::resolver::query> /* query */
    );


    static void connectHandler(
        const boost::system::error_code& error,
        ClientnodeMachine::CountedReference cm,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator
    );


    boost::statechart::result react(const EvtConnectReport& evt);
    boost::statechart::result react(const EvtDisconnectRequest&);
    boost::statechart::result react(const EvtSendMsg<NearUserMessage>&);
    boost::statechart::result react(const EvtConnectRequest& evt);
};


struct StateConnected :
    public boost::statechart::state<StateConnected, ClientnodeMachine>
{
    /** State reactions. */
    typedef boost::mpl::list<
        boost::statechart::custom_reaction<EvtDisconnectRequest>,
        boost::statechart::custom_reaction<EvtSendMsg<NearUserMessage>>,
        boost::statechart::custom_reaction<EvtDisconnected>,
        boost::statechart::custom_reaction<EvtRcvdMessage<SerializedData>>,
        boost::statechart::custom_reaction<EvtConnectRequest>
    > reactions;

    /** Constructor. To be used only by Boost.Statechart classes. */
    StateConnected(my_context ctx);

    boost::statechart::result react(const EvtDisconnectRequest&);
    boost::statechart::result react(const EvtSendMsg<NearUserMessage>&);
    boost::statechart::result react(const EvtDisconnected& evt);
    boost::statechart::result react(const EvtRcvdMessage<SerializedData>& evt);
    boost::statechart::result react(const EvtConnectRequest& evt);

    static void writeHandler(
        const boost::system::error_code& error,
        std::size_t bytes_transferred,
        ClientnodeMachine::CountedReference cm,
        std::shared_ptr<byte_traits::byte_sequence> data
    );

    static void receiveSegmentationHeaderHandler(
        const boost::system::error_code& error,
        std::size_t bytes_transferred,
        ClientnodeMachine::CountedReference cm,
        byte_traits::byte_t rcvbuf[SegmentationLayerBase::header_length]
    );

    static void receiveSegmentationBodyHandler(
        const boost::system::error_code& error,
        std::size_t bytes_transferred,
        ClientnodeMachine::CountedReference cm,
        std::shared_ptr<byte_traits::byte_sequence> rcvbuf
    );

};

// this function is declared in clientnode.hpp and defined in clientnode.hpp
extern void catchThread(boost::thread& thread, unsigned threadwait_ms);


} // namespace clientnode

/**@}*/ // defgroup proto_machine

} // namespace nuke_ms


#endif // ifndef STATEMACHINE_HPP
