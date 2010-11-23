
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQNET_BUFFER_CONNECTION_H
#define EQNET_BUFFER_CONNECTION_H

#include <eq/net/connection.h>   // base class
#include <eq/base/buffer.h>      // member

namespace eq
{
namespace net
{
    /**
     * A proxy connection buffering outgoing data into a memory region.
     */
    class BufferConnection : public Connection
    {
    public:
        EQNET_API BufferConnection();
        EQNET_API virtual ~BufferConnection();

        EQNET_API void sendBuffer( ConnectionPtr connection );

        EQNET_API uint64_t getSize() const { return _buffer.getSize(); }

    protected:
        virtual void readNB( void*, const uint64_t )
            { EQDONTCALL; }
        virtual int64_t readSync( void*, const uint64_t, const bool )
            { EQDONTCALL; return -1; }
        EQNET_API virtual int64_t write( const void* buffer,
                                         const uint64_t bytes );

        virtual Notifier getNotifier() const { EQDONTCALL; return 0; }

    private:
        mutable base::Bufferb _buffer;
    };
}
}

#endif //EQNET_BUFFER_CONNECTION_H
