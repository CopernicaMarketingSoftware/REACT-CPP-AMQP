/**
 *  MyConnection.cpp
 *
 *  @copyright 2014 Copernica BV
 */

#include "myconnection.h"

/**
 *  Constructor
 */
MyConnection::MyConnection(React::Loop *loop, const char *ip) :
    _loop(loop),
    _connection(loop, this, ip, 5672, AMQP::Login("guest", "guest"), "/"),
    _channel(nullptr)
{}

/**
 *  Method that is called when the connection succeeded
 *  @param  connection  Pointer to the connection
 */
void MyConnection::onConnected(React::AMQP::Connection *connection)
{
    // report connection
    std::cout << "connected" << std::endl;

    // create a new channel
    _channel = std::unique_ptr<React::AMQP::Channel>(new React::AMQP::Channel(&_connection));

    // watch for the channel becoming ready
    _channel->onReady([this]() {
        // show that we are ready
        std::cout << "AMQP channel ready, id: " << (int) _channel->id() << std::endl;
    });

    // and of course for channel errors
    _channel->onError([this](const std::string& message) {
        // inform the user of the error
        std::cerr << "AMQP channel error on channel " << _channel->id() << ": " << message << std::endl;

        // delete the channel
        _channel = nullptr;

        // close the connection
        _connection.close();
    });

    // declare a queue and let us know when it succeeds
    _channel->declareQueue("my_queue").onSuccess([](const std::string &name, uint32_t messageCount, uint32_t consumerCount){
        // queue was successfully declared
        std::cout << "AMQP Queue declared with name '" << name << "', " << messageCount << " messages and " << consumerCount << " consumer" << std::endl;
    });

    // also declare an exchange
    _channel->declareExchange("my_exchange", AMQP::direct).onSuccess([]() {
        // exchange successfully declared
        std::cout << "AMQP exchange declared" << std::endl;
    });

    // bind the queue to the exchange
    _channel->bindQueue("my_exchange", "my_queue", "key").onSuccess([]() {
        // queue successfully bound to exchange
        std::cout << "AMQP Queue bound" << std::endl;
    });

    // set quality of service
    _channel->setQos(1).onSuccess([]() {
        // quality of service successfully set
        std::cout << "AMQP Quality of Service set" << std::endl;
    });

    // publish a message to the exchange
    if (!_channel->publish("my_exchange", "key", "my_message"))
    {
        // we could not publish the message, something is wrong somewhere
        std::cerr << "Unable to publish message" << std::endl;

        // close the channel
        _channel->close().onSuccess([this]() {
            // also close the connection
            _connection.close();
        });
    }

    // consume the message we just published
    _channel->consume("my_queue", "my_consumer", AMQP::exclusive)
    .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
        // show the message data
        std::cout << "AMQP consumed: " << message.message() << std::endl;

        // ack the message
        _channel->ack(deliveryTag);

        // and stop consuming (there is only one message anyways)
        _channel->cancel("my_consumer").onSuccess([](const std::string& tag) {
            // we successfully stopped consuming
            std::cout << "Stopped consuming under tag " << tag << std::endl;
        });

        // unbind the queue again
        _channel->unbindQueue("my_exchange", "my_queue", "key").onSuccess([]() {
            // queueu successfully unbound
            std::cout << "Queue unbound" << std::endl;
        });

        // the queue should now be empty, so we can delete it
        _channel->removeQueue("my_queue").onSuccess([](uint32_t messageCount) {
            // queue was removed, it should have been empty, so messageCount should be 0
            if (messageCount) std::cerr << "Removed queue which should have been empty but contained " << messageCount << " messages" << std::endl;

            // no messages is the expected behavior
            else std::cout << "Queue removed" << std::endl;
        });

        // also remove the exchange
        _channel->removeExchange("my_exchange").onSuccess([]() {
            // exchange was successfully removed
            std::cout << "Removed exchange" << std::endl;
        });

        // everything done, close the channel
        _channel->close().onSuccess([this]() {
            // channel was closed
            std::cout << "Channel closed" << std::endl;

            // close the connection too
            _connection.close();
        });
    })
    .onSuccess([]() {
        // consumer was started
        std::cout << "Started consuming" << std::endl;
    });
}

/**
 *  Method that is called when the socket is closed (as a result of a TcpSocket::close() call)
 *  @param  connection  Pointer to the connection
 */
void MyConnection::onClosed(React::AMQP::Connection *connection)
{
    // show
    std::cout << "myconnection closed" << std::endl;

    // close the channel and connection
    _channel = nullptr;

    // stop the loop
    _loop->stop();
}

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
void MyConnection::onError(React::AMQP::Connection *connection, const char *message)
{
    // report error
    std::cout << "AMQP Connection error: " << message << std::endl;
}
