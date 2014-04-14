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
class ConnectionHandler : public ::AMQP::ConnectionHandler
{
private:
    /**
     *  Send data over the network
     *
     *  @param  connection  the connection creating the output
     *  @param  buffer      the data to send
     *  @param  size        the size of the data to send
     */
    void onData(::AMQP::Connection *connection, const char *buffer, size_t size) override;

    /**
     *  Handle the event of the connection entering error state
     *
     *  @param  connection  the connection entering error state
     *  @param  message     description of the problem
     */
    void onError(::AMQP::Connection *connection, const std::string& message) override;

    /**
     *  Handle the event of a successful login
     *
     *  @param  connection  the connection now logged in
     */
    void onConnected(::AMQP::Connection *connection) override;

    /**
     *  Handle the event of the connection being closed
     *
     *  @param  connection  the connection now closed
     */
    void onClosed(::AMQP::Connection *connection) override;
public:
    /**
     *  Method that will be called in the event of an error
     *
     *  @param  connection  the connection that triggered an error
     *  @param  message     a description of the error
     */
    virtual void onError(Connection *connection, const std::string& message) {}

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
