/**
 *  ConnectionHandler.h
 *
 *  Handler for connection events.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace React { namespace AMQP {

// forward declaration
class Connection;

/**
 *  Connection handler class
 */
class ConnectionHandler
{
public:
    /**
     *  Method that will be called in the event of an error
     *
     *  @param  connection  the connection that triggered an error
     *  @param  message     a description of the error
     */
    virtual void onError(Connection *connection, const char *message) {}

    /**
     *  Method that will be called once the login is completed
     *
     *  @param  connection  the connection now logged in
     */
    virtual void onConnected(Connection *connection) {}

    /**
     *  Method that will be called once the connection is closed
     *
     *  @param  connection  the connection now closed
     */
    virtual void onClosed(Connection *connection) {}
};

/**
 *  End namespace
 */
}}
