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
    std::unique_ptr<React::Tcp::Connection> _socket;

    /**
     *  Buffer for incoming data
     */
    std::unique_ptr<React::Tcp::In<>> _input;

    /**
     *  Buffer for outgoing data
     */
    std::string _output;

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
     *  Install a read handler
     */
    void checkReadable()
    {
        // parse incoming data
        _input->onData([this](const void *buf, size_t size) {
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
     *  Install a write handler
     */
    void checkWritable()
    {
        // we can only install a handler when a socket is available
        if (!_output.empty() && _socket)
        {
            // push the data when writable
            _socket->onWritable([this]() {
                // send all available bytes to the socket
                ssize_t result = _socket->send(_output.c_str(), _output.size());

                // did we write all data at once?
                if (result == _output.size())
                {
                    // clear the buffer, no more data to send
                    _output.clear();
                    return false;
                }

                // remove the bytes that were sent from the buffer
                _output.erase(0, result);

                // more data remains to be sent
                return true;
            });
        }
    }

    /**
     *  Connect to one of the given IP addresses
     *
     *  Any existing connection is simply thrown away with no regard for safety. This
     *  is only to be used for making an initial connection to the AMQP broker.
     *
     *  @param  ips     list with ip addresses to try
     *  @param  port    port to try connecting on
     */
    void connect(React::Dns::IpResult&& ips, uint16_t port)
    {
        // move the list into a pointer to avoid
        // copying and keep it from being const
        auto list = std::make_shared<React::Dns::IpResult>(std::move(ips));

        // retrieve the first address and remove from the list
        auto address = *list->cbegin();
        list->erase(list->cbegin());

        // try to connect to the ip address
        try
        {
            // create a new socket
            _socket.reset(new React::Tcp::Connection(_loop, address.ip(), port));
            _input.reset(new React::Tcp::In<>(_socket.get()));

            // wait for the connection to be established
            _socket->onConnected([this, list, port](const char *error) {
                // check for errors
                if (error)
                {
                    // try other addresses or fail if nothing else is available
                    if (!list->empty()) connect(std::move(*list), port);
                    else _handler->onError(this, error);

                    // nothing else to do
                    return;
                }

                // check for incoming and outgoing data data
                checkReadable();
                checkWritable();
            });
        }
        catch (const React::Exception& exception)
        {
            // try other addresses or fail if nothing else is available
            if (!list->empty()) connect(std::move(*list), port);
            else _handler->onError(this, exception.what());
        }
    }

    /**
     *  Send data over the network
     *
     *  @param  connection  the connection creating the output
     *  @param  buffer      the data to send
     *  @param  size        the size of the data to send
     */
    void onData(::AMQP::Connection *connection, const char *buffer, size_t size) override
    {
        // add the data to the output buffer
        _output.append(buffer, size);

        // install the write handler
        checkWritable();
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
        if (_handler) _handler->onError(this, message);
    }

    /**
     *  Handle the event of a successful login
     *
     *  @param  connection  the connection now logged in
     */
    void onConnected(::AMQP::Connection *connection) override
    {
        // send to the handler
        if (_handler) _handler->onConnected(this);
    }

    /**
     *  Handle the event of the connection being closed
     *
     *  @param  connection  the connection now closed
     */
    void onClosed(::AMQP::Connection *connection) override
    {
        // no longer need the socket or the input
        _socket.reset();
        _input.reset();

        // send to the handler
        if (_handler) _handler->onClosed(this);
    }
public:
    /**
     *  Constructor
     *
     *  @param  loop        the event loop to use for IO
     *  @param  handler     the handler that receives notifications
     *  @param  ip          the ip address to connect to
     *  @param  port        the port to connect to
     *  @param  login       the username and password combination to use
     *  @param  vhost       the vhost to connect to
     */
    Connection(React::Loop *loop, React::AMQP::ConnectionHandler *handler, const Net::Ip& ip, uint16_t port, const ::AMQP::Login& login, const std::string& vhost) :
        _loop(loop),
        _handler(handler),
        _socket(new React::Tcp::Connection(loop, ip, port)),
        _input(new React::Tcp::In<>(_socket.get())),
        _connection(this, login, vhost)
    {
        // wait for the connection to be established
        if (_handler) _socket->onConnected([this](const char *error) {
            // check for errors
            if (error) _handler->onError(this, error);
        });

        // check for incoming and outgoing data
        checkReadable();
        checkWritable();
    }

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
    Connection(React::Loop *loop, React::AMQP::ConnectionHandler *handler, const std::string& host, uint16_t port, const ::AMQP::Login& login, const std::string& vhost) :
        _loop(loop),
        _handler(handler),
        _connection(this, login, vhost)
    {
        // create a resolver to find the IP address
        auto resolver = std::make_shared<React::Dns::Resolver>(loop);

        // resolve the given hostname
        resolver->ip(host, [this, host, port, loop, resolver](React::Dns::IpResult&& ips, const char *error) {
            // check for errors
            if (error)
            {
                if (_handler) _handler->onError(this, error);
                return;
            }

            // check whether any addresses were found
            if (ips.empty())
            {
                if (_handler) _handler->onError(this, std::string("No records found for host ").append(host).c_str());
                return;
            }

            // connect to one of the available addresses
            connect(std::move(ips), port);
        });
    }

    /**
     *  Close the connection
     */
    bool close()
    {
        return _connection.close();
    }

    /**
     *  Retrieve the amount of active channels
     *  @return std::size_t
     */
    std::size_t channels() const
    {
        return _connection.channels();
    }

    // we are friends with the handler and the channel
    friend class ConnectionHandler;
    friend class Channel;
};

/**
 *  End namespace
 */
}}
