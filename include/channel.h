/**
 *  Channel.h
 *
 *  Wrapper class for the AMQP channel
 *
 *  @copyright 2014 Copernica BV
 */

#pragma once

/**
 *  Set up namespace
 */
namespace React { namespace AMQP {

/**
 *  Channel class
 */
class Channel : public ::AMQP::Channel
{
public:
    /**
     *  Constructor
     *
     *  @param  connection  connection to open a channel over
     */
    Channel(Connection *connection) : ::AMQP::Channel(&connection->_connection)
    {}
};

/**
 *  End namespace
 */
}}
