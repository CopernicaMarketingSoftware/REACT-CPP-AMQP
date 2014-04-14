/**
 *  Main.cpp
 *
 *  Test program
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Local libraries
 */
#include "myconnection.h"

/**
 *  Main procedure
 *  @param  argc
 *  @param  argv
 *  @return int
 */
int main(int argc, const char *argv[])
{
    // need an ip
    if (argc != 2)
    {
        // report error
        std::cerr << "usage: " << argv[0] << " <ip>" << std::endl;

        // done
        return -1;
    }
    else
    {
        // create the loop
        React::MainLoop loop;

        // create connection
        MyConnection connection(&loop, argv[1]);

        // start the main event loop
        loop.run();

        // done
        return 0;
    }
}

