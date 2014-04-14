/**
 *  Connection.h
 *
 *  Main file holding the connection to RabbitMQ
 *
 *  @copyright 2014 Copernica BV
 */

#pragma once

/**
 *  Set up namespace
 */
namespace React { namespace AMQP {

// forward declaration
class ConnectionHandler;

/**
 *  Connection class
 */
class Connection :
    public  ::AMQP::Connection
{
private:
    /**
     *  The event loop to use for IO
     */
    React::Loop *_loop;

    /**
     *  The socket connection to the server
     */
    React::Tcp::Connection _socket;

    /**
     *  The output buffer for sending data
     */
    React::Tcp::Out _output;

    /**
     *  The handler that must be notified
     */
    React::AMQP::ConnectionHandler *_handler;
public:
    /**
     *  Constructor
     *
     *  @param  loop        the event loop to use for IO
     *  @param  handler     the handler that receives notifications
     *  @param  host        the host to connect to
     *  @param  port        the port to connect to
     *  @param  login       the username and password combination to use
     *  @param  vhost       the vhost to connect to
     */
    Connection(React::Loop *loop, React::AMQP::ConnectionHandler *handler, const Net::Ip& host, uint16_t port, const ::AMQP::Login& login, const std::string& vhost) :
        ::AMQP::Connection(handler, login, vhost),
        _loop(loop),
        _socket(loop, host, port),
        _output(&_socket),
        _handler(handler)
    {
        std::cout << "Installing data handler using onData" << std::endl;

        // we have to check for a connection
        _socket.onConnected([](const char *error) {
            if (error) std::cerr << "Could not connect: " << error << std::endl;
            else std::cout << "Now connected" << std::endl;
        });

        // parse incoming data
        _socket.onData([this](const void *buf, size_t size) {
            std::cout << ">> " << std::string(static_cast<const char *>(buf), size) << std::endl;

            // pass to AMQP (TODO: handle chunked data)
            parse(static_cast<const char *>(buf), size);

            // we would like to continue receiving data
            return true;
        });

        _socket.onLost([]() {
            std::cout << "Lost connection to AMQP :(" << std::endl;
        });

        std::cout << "Data handler installed" << std::endl;
    }

    // we are friends with the handler
    friend class ConnectionHandler;
};

/**
 *  End namespace
 */
}}
