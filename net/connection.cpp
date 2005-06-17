
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connection.h"

#include "pipeConnection.h"
#include "socketConnection.h"

#include <eq/base/log.h>

#include <errno.h>

using namespace eqNet;
using namespace std;

Connection::Connection()
        : _state( STATE_CLOSED )
{
}

Connection* Connection::create( ConnectionDescription &description )
{
    Connection* connection;

    switch( description.protocol )
    {
        case Network::PROTO_TCPIP:
            connection = new SocketConnection();
            break;

        case Network::PROTO_PIPE:
            connection = new PipeConnection();
            break;

        default:
            WARN << "Protocol not implemented" << endl;
            return NULL;
    }

    connection->_description = description;
    return connection;
}

Connection* Connection::select( const std::vector<Connection*> &connections, 
    const int timeout, short &event )
{
    static const int events       = POLLIN; // | POLLPRI;
    const int        nConnections = connections.size();

    // prepare pollfd set
    pollfd*         pollFDs = (pollfd*)alloca( nConnections * sizeof( pollfd ));
    for( size_t i=0; i<nConnections; i++ )
    {
        pollFDs[i].fd = connections[i]->getReadFD();
        if( pollFDs[i].fd == -1 )
        {
            WARN << "Cannot select connection " << i
                 << ", connection does not use file descriptor" << endl;
            event = POLLERR;
            return NULL;
        }

        pollFDs[i].events  = events;
        pollFDs[i].revents = 0;

        char b;
        INFO <<  connections[i]->read( &b, 1) << endl;
        INFO << "Connection " << i << " fd " << pollFDs[i].fd << " ev "
             << pollFDs[i].events << endl;
    }

    // poll for a result
    const int ret = poll( pollFDs, nConnections, timeout );
    switch( ret )
    {
        case 0: // TIMEOUT
            event = 0;
            return NULL;

        case -1: // ERROR
            event = POLLERR;
            WARN << "Error during poll(): " << strerror( errno ) << endl;
            return NULL;

        default: // SUCCESS
            for( size_t i=0; i<nConnections; i++ )
            {
                if( pollFDs[i].revents == 0 )
                    continue;

                event = pollFDs[i].revents;
                INFO << "selected connection #" << i << " of " << nConnections
                     << ", event " << event << endl;
                return connections[i];
            }
            WARN << "Error: Unreachable code" << endl;
            event = POLLERR;
            return NULL;
    }
}
