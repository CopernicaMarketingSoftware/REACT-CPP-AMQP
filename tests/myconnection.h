/**
 *  MyConnection.h
 *
 *  Our own test implementation for a connection handler
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Required external libraries
 */
#include <reactcpp/amqp.h>

/**
 *  Class definition
 */
class MyConnection :
    public React::AMQP::ConnectionHandler
{
private:
    /**
     *  The event loop to work with
     */
    React::Loop *_loop;

    /**
     *  The AMQP connection
     *  @var    Connection
     */
    React::AMQP::Connection _connection;

    /**
     *  The AMQP channel
     *  @var    Channel
     */
    std::unique_ptr<React::AMQP::Channel> _channel;

    /**
     *  Method that is called when the connection to AMQP was closed
     *  @param  connection  pointer to connection object
     */
    void onClosed(React::AMQP::Connection *connection) override;

    /**
     *  When the connection ends up in an error state this method is called.
     *  This happens when data comes in that does not match the AMQP protocol
     *
     *  After this method is called, the connection no longer is in a valid
     *  state and can be used. In normal circumstances this method is not called.
     *
     *  @param  connection      The connection that entered the error state
     *  @param  message         Error message
     */
    void onError(React::AMQP::Connection *connection, const char *message) override;

    /**
     *  Method that is called when the login attempt succeeded. After this method
     *  was called, the connection is ready to use
     *
     *  @param  connection      The connection that can now be used
     */
    void onConnected(React::AMQP::Connection *connection) override;

public:
    /**
     *  Constructor
     *  @param  ip
     */
    MyConnection(React::Loop *loop, const std::string& host);
};

