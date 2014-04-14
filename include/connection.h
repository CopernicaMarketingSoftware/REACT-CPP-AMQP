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
    private React::Tcp::Connection,
    private React::Tcp::Out,
    public  ::AMQP::Connection
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
     *  Buffer for incoming data that cannot be
     *  handled straight-away (e.g. half frames)
     */
    std::string _buffer;
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
        React::Tcp::Connection(loop, host, port),
        React::Tcp::Out(this),
        ::AMQP::Connection(handler, login, vhost),
        _loop(loop),
        _handler(handler)
    {
        // wait for the connection to be established
        onConnected([this](const char *error) {
            // check for errors
            if (error) _handler->onError(this, error);
        });

        // parse incoming data
        onData([this](const void *buf, size_t size) {
            // do we have an existing buffer from before?
            if (!_buffer.empty())
            {
                // add the new data to the buffer
                _buffer.append(static_cast<const char *>(buf), size);

                // parse all data we have now and remove what is parsed
                _buffer.erase(0, parse(_buffer.c_str(), _buffer.size()));
            }
            else
            {
                // pass to AMQP
                auto parsed = parse(static_cast<const char *>(buf), size);

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
        return this->::AMQP::Connection::close();
    }

    // we are friends with the handler
    friend class ConnectionHandler;
};

/**
 *  End namespace
 */
}}
