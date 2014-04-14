/**
 *  ConnectionHandler.cpp
 *
 *  Handler for connection events.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Dependencies
 */
#include <amqpcpp.h>
#include <reactcpp.h>

/**
 *  Other include files
 */
#include <../include/connectionhandler.h>
#include <../include/connection.h>

/**
 *  Set up namespace
 */
namespace React { namespace AMQP {

/**
 *  Set a function to be executed after a given timeout.
 *
 *  @param  connection  the connection triggering the timeout
 *  @param  timeout     number of seconds to wait
 *  @param  callback    function to execute once time runs out
 */
void ConnectionHandler::setTimeout(::AMQP::Connection *connection, double seconds, const std::function<void()>& callback)
{
    // set timeout on the loop
    static_cast<Connection*>(connection)->_loop->onTimeout(seconds, callback);
}

/**
 *  Send data over the network
 *
 *  @param  connection  the connection creating the output
 *  @param  buffer      the data to send
 *  @param  size        the size of the data to send
 */
void ConnectionHandler::onData(::AMQP::Connection *connection, const char *buffer, size_t size)
{
    // write data to the socket
    static_cast<Connection*>(connection)->::React::Tcp::Out::send(buffer, size);
}

/**
 *  Handle the event of the connection entering error state
 *
 *  @param  connection  the connection entering error state
 *  @param  message     description of the problem
 */
void ConnectionHandler::onError(::AMQP::Connection *connection, const std::string& message)
{
    // cast to the correct connection and pass on
    onError(static_cast<Connection*>(connection), message);
}

/**
 *  Handle the event of a successful login
 *
 *  @param  connection  the connection now logged in
 */
void ConnectionHandler::onConnected(::AMQP::Connection *connection)
{
    // cast to the correct connection and pass on
    onConnected(static_cast<Connection*>(connection));
}

/**
 *  Handle the event of the connection being closed
 *
 *  @param  connection  the connection now closed
 */
void ConnectionHandler::onClosed(::AMQP::Connection *connection)
{
    // the socket should be closed now
    static_cast<Connection*>(connection)->::React::Tcp::Connection::close();

    // cast to the correct connection and pass on
    onClosed(static_cast<Connection*>(connection));
}

/**
 *  End namespace
 */
}}
