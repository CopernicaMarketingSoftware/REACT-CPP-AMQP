REACT-CPP-AMQP
==============

Library to work with the RabbitMQ broker using the REACT-CPP event loop

This library is a simple wrapper around the AMQP-CPP library, utilizing
the REACT-CPP library for socket IO. To use this library, you will need
at least the REACT-CPP and AMQP-CPP libraries.

````c++
#include <reactcpp/amqp.h>

/**
 *  Connection handler
 */
class Handler : public React::AMQP::ConnectionHandler
{
    /**
     *  Method that will be called in the event of an error
     *
     *  @param  connection  the connection that triggered an error
     *  @param  message     a description of the error
     */
    virtual void onError(Connection *connection, const std::string& message)
    {
        // report the error
        std:cerr << "Connection error: " << message << std::endl;
    }

    /**
     *  Method that will be called once the login is completed
     *
     *  @param  connection  the connection now logged in
     */
    virtual void onConnected(Connection *connection)
    {
        // report that we are now connected
        std::cout << "Connected succesfully" << std::endl;

        // and close the connection again
        connection->close();
    }

    /**
     *  Method that will be called once the connection is closed
     *
     *  @param  connection  the connection now closed
     */
    virtual void onClosed(Connection *connection) {
        // report that the connection closed down
        std::cout << "Connection to AMQP closed" << std::endl;
    }
};

/**
 *  Main procedure
 */
int main(int argc, const char *argv[])
{
    // check for correct arguments
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <rabbitmq ip address>" << std::endl;
        return 1;
    }

    // the loop to work with
    React::MainLoop loop;

    // connection handler
    Handler handler;

    // connection to RabbitMQ
    React::AMQP::Connection connection(&loop, &handler, argv[1], 5672, AMQP::Login("guest", "guest"), "/");

    // start the loop
    loop.run();
}
````

For further information on how to work with the AMQP library, please see the AMQP-CPP documentation.
