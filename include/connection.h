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
class Channel;

/**
 *  Connection class
 */
class Connection : private ::AMQP::ConnectionHandler
{
private:
    /**
     *  The event loop to use for IO
     */
    React::Loop *_loop;

    /**
     *  The handler that must be notified
     */
    React::AMQP::ConnectionHandler *_handler;

    /**
     *  Socket connection to AMQP
     */
    React::Tcp::Connection _socket;

    /**
     *  Buffer for incoming data
     */
    React::Tcp::In<> _input;

    /**
     *  Buffer for outgoing data
     */
    React::Tcp::Out _output;

    /**
     *  Connection to AMQP
     */
    ::AMQP::Connection _connection;

    /**
     *  Buffer for incoming data that cannot be
     *  handled straight-away (e.g. half frames)
     */
    std::string _buffer;

    /**
     *  Send data over the network
     *
     *  @param  connection  the connection creating the output
     *  @param  buffer      the data to send
     *  @param  size        the size of the data to send
     */
    void onData(::AMQP::Connection *connection, const char *buffer, size_t size) override
    {
        // send data over the output buffer
        _output.send(buffer, size);
    }

    /**
     *  Handle the event of the connection entering error state
     *
     *  @param  connection  the connection entering error state
     *  @param  message     description of the problem
     */
    void onError(::AMQP::Connection *connection, const char *message) override
    {
        // send to the handler
        _handler->onError(this, message);
    }

    /**
     *  Handle the event of a successful login
     *
     *  @param  connection  the connection now logged in
     */
    void onConnected(::AMQP::Connection *connection) override
    {
        // send to the handler
        _handler->onConnected(this);
    }

    /**
     *  Handle the event of the connection being closed
     *
     *  @param  connection  the connection now closed
     */
    void onClosed(::AMQP::Connection *connection) override
    {
        // send to the handler
        _handler->onClosed(this);
    }
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
        ::AMQP::ConnectionHandler(),
        _loop(loop),
        _handler(handler),
        _socket(loop, host, port),
        _input(&_socket),
        _output(&_socket),
        _connection(this, login, vhost)
    {
        // wait for the connection to be established
        _socket.onConnected([this](const char *error) {
            // check for errors
            if (error) _handler->onError(this, error);
        });

        // parse incoming data
        _input.onData([this](const void *buf, size_t size) {
            // do we have an existing buffer from before?
            if (!_buffer.empty())
            {
                // add the new data to the buffer
                _buffer.append(static_cast<const char *>(buf), size);

                // parse all data we have now and remove what is parsed
                _buffer.erase(0, _connection.parse(_buffer.c_str(), _buffer.size()));
            }
            else
            {
                // pass to AMQP
                auto parsed = _connection.parse(static_cast<const char *>(buf), size);

                // store the remaining bytes in the buffer
                _buffer.append(static_cast<const char *>(buf) + parsed, size - parsed);
            }

            // we would like to continue receiving data
            return true;
        });
    }

    /**
     *  Close the connection
     */
    bool close()
    {
        return _connection.close();
    }

    // we are friends with the handler and the channel
    friend class ConnectionHandler;
    friend class Channel;
};

/**
 *  End namespace
 */
}}
