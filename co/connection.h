
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef CO_CONNECTION_H
#define CO_CONNECTION_H

#include <co/api.h>
#include <co/packets.h>               // used in inline method
#include <co/types.h>

#include <lunchbox/referenced.h>   // base class

#include <sys/types.h>
#include <string.h>
#include <vector>

#ifdef _WIN32
#  define EQ_DEFAULT_PORT (4242)
#else
#  define EQ_DEFAULT_PORT (4242 + getuid())
#endif

namespace co
{
namespace detail { class Connection; }

    /**
     * An interface definition for communication between hosts.
     *
     * Connections are stream-oriented communication lines. The parameters of a
     * Connection are described in a ConnectionDescription, which is used in
     * listen() and connect(). A Connection has a Connection::State, which
     * changes when calling listen(), connect() or close(), or whenever the
     * underlying connection is closed by the operating system.
     *
     * The Connection class defines the interface for connections, various
     * derived classes implement it for the low-level communication protocols,
     * e.g., SocketConnection for TCP/IP. An implementation may not implement
     * all the functionality defined in this interface.
     *
     * The Connection is used reference-counted in co, since it has multiple
     * owners, such as the ConnectionSet and Node.
     */
    class Connection : public lunchbox::Referenced, public lunchbox::NonCopyable
    {
    public:
        enum State //! The current state of the Connection @version 1.0
        {
            STATE_CLOSED,     //!< Closed, initial state
            STATE_CONNECTING, //!< A connect() or listen() is in progress
            STATE_CONNECTED,  //!< The connection has been connected and is open
            STATE_LISTENING,  //!< The connection is listening for connects
            STATE_CLOSING     //!< A close() is in progress
        };

        /**
         * Create a new connection.
         *
         * This factory method creates a new concrete connection for the
         * requested type. The description is set on the created Connection.
         *
         * @param desc the connection parameters.
         * @return the connection.
         * @version 1.0
         */
        CO_API static ConnectionPtr create( ConnectionDescriptionPtr desc );

        /** @name Data Access */
        //@{
        /** @return the State of this connection. @version 1.0 */
        CO_API State getState() const;

        /** @return true if the connection is closed. @version 1.0 */
        bool isClosed() const { return getState() == STATE_CLOSED; }

        /** @return true if the connection is about to close. @version 1.0 */
        bool isClosing() const { return getState() == STATE_CLOSING; }

        /** @return true if the connection is connected. @version 1.0 */
        bool isConnected() const { return getState() == STATE_CONNECTED; }

        /** @return true if the connection is listening. @version 1.0 */
        bool isListening() const { return getState() == STATE_LISTENING; }

        /** @return the description for this connection. @version 1.0 */
        CO_API ConstConnectionDescriptionPtr getDescription() const;

        /** @internal */
        bool operator == ( const Connection& rhs ) const;
        //@}

        /** @name Connection State Changes */
        //@{
        /**
         * Connect to the remote peer.
         *
         * The ConnectionDescription of this connection is used to identify the
         * peer's parameters.
         *
         * @return true if the connection was successfully connected, false
         *         if not.
         * @version 1.0
         */
        virtual bool connect() { return false; }

        /**
         * Put the connection into the listening state.
         *
         * The ConnectionDescription of this connection is used to identify the
         * listening parameters.
         *
         * @return true if the connection is listening for new incoming
         *         connections, false if not.
         * @version 1.0
         */
        virtual bool listen() { return false; }

        /** Close a connected or listening connection. @version 1.0 */
        virtual void close() {}
        //@}

        /** @name Listener Interface */
        //@{
        /** @internal Add a listener for connection state changes. */
        void addListener( ConnectionListener* listener );

        /** @internal Remove a listener for connection state changes. */
        void removeListener( ConnectionListener* listener );
        //@}

        /** @name Asynchronous accept */
        //@{
        /**
         * Start an accept operation.
         *
         * This method returns immediately. The Notifier will signal a new
         * connection request, upon which acceptSync() should be used to finish
         * the accept operation.
         *
         * @sa acceptSync()
         * @version 1.0
         */
        virtual void acceptNB() { LBUNIMPLEMENTED; }

        /**
         * Complete an accept operation.
         *
         * @return the new connection, 0 on error.
         * @version 1.0
         */
        virtual ConnectionPtr acceptSync() { LBUNIMPLEMENTED; return 0; }
        //@}

        /** @name Asynchronous read */
        //@{
        /**
         * Start a read operation on the connection.
         *
         * This function returns immediately. The Notifier will signal data
         * availability, upon which recvSync() should be used to finish the
         * operation.
         *
         * @param buffer the buffer receiving the data.
         * @param bytes the number of bytes to read.
         * @sa recvSync()
         * @version 1.0
         */
        CO_API void recvNB( void* buffer, const uint64_t bytes );

        /**
         * Finish reading data from the connection.
         *
         * This function may block even if data availability was signaled, i.e.,
         * when only a part of the data requested has been received. The buffer
         * and bytes return value pointers can be 0. This method uses readNB()
         * and readSync() to fill a buffer, potentially by using multiple reads.
         *
         * @param buffer return value, the buffer pointer passed to recvNB().
         * @param bytes return value, the number of bytes read.
         * @param block internal WAR parameter, do not use unless you know
         *              exactly why.
         * @return true if all requested data has been read, false otherwise.
         * @version 1.0
         */
        CO_API bool recvSync( void** buffer, uint64_t* bytes,
                              const bool block = true );

        void resetRecvData( void** buffer, uint64_t* bytes ); //!< @internal
        //@}

        /** @name Synchronous write to the connection */
        //@{
        /**
         * Send data using the connection.
         *
         * A send may be performed using multiple write() operations. For
         * thread-safe sending from multiple threads it is therefore crucial to
         * protect the send() operation internally. If the connection is not
         * already locked externally, it will use an internal mutex.
         *
         * @param buffer the buffer containing the message.
         * @param bytes the number of bytes to send.
         * @param isLocked true if the connection is locked externally.
         * @return true if all data has been read, false if not.
         * @sa lockSend(), unlockSend()
         * @version 1.0
         */
        CO_API bool send( const void* buffer, const uint64_t bytes,
                          const bool isLocked = false );

        /** Lock the connection, no other thread can send data. @version 1.0 */
        CO_API void lockSend() const;

        /** Unlock the connection. @version 1.0 */
        CO_API void unlockSend() const;

        /**
         * Sends a packaged message using the connection.
         *
         * @param packet the message packet.
         * @return true if all data has been read, false if not.
         * @version 1.0
         */
        bool send( const Packet& packet )
            { return send( &packet, packet.size ); }

        /**
         * Sends a packaged message including a string using the connection.
         *
         * The packet has to define a 8-byte-aligned, 8-char array at the end
         * of the packet. When the packet is sent the whole string is appended
         * to the packet, so that the receiver has to do nothing special to
         * receive and use the full packet.
         *
         * @param packet the message packet.
         * @param string the string.
         * @return true if all data has been read, false if not.
         * @version 1.0
         */
        bool send( Packet& packet, const std::string& string )
            { return send( packet, string.c_str(), string.size()+1 ); }

        /**
         * Sends a packaged message including additional data.
         *
         * The last item of the packet has to be able to hold one item or eight
         * bytes of the data, whatever is bigger.
         *
         * @param packet the message packet.
         * @param data the vector containing the data.
         * @return true if all data has been read, false if not.
         * @version 1.0
         */
        template< typename T >
        bool send( Packet& packet, const std::vector<T>& data );

        /**
         * Sends a packaged message including additional data.
         *
         * @param packet the message packet.
         * @param data the data.
         * @param size the data size in bytes.
         * @return true if all data has been read, false if not.
         * @version 1.0
         */
        CO_API bool send( Packet& packet, const void* data,
                          const uint64_t size );

        /**
         * Sends a packaged message to multiple connections.
         *
         * @param connections The connections.
         * @param packet      the message packet.
         * @param isLocked true if the connection is locked externally.
         * @return true if the packet was sent successfully to all connections.
         * @version 1.0
         */
        static CO_API bool send( const Connections& connections,
                                 const Packet& packet,
                                 const bool isLocked = false );
        /**
         * Sends a packaged message including additional data to multiple
         * connections.
         *
         * @param connections The connections.
         * @param packet the message packet.
         * @param data the data.
         * @param size the data size in bytes.
         * @param isLocked true if the connection is locked externally.
         * @return true if the packet was sent successfully to all receivers.
         * @version 1.0
         */
        static CO_API bool send( const Connections& connections,
                                 Packet& packet, const void* data,
                                 const uint64_t size,
                                 const bool isLocked = false );
        /**
         * Sends a packaged message including additional, multiple data items to
         * multiple connections.
         *
         * Automatically locks each individual connection during send. The data
         * items are appended to the packet individually as a 8-byte size token
         * followed by size bytes data. The packet.size is updated to the size
         * of all data send (packet + sizes + items).
         *
         * @param connections The connections.
         * @param packet the message packet.
         * @param items a pointer array to the data items.
         * @param itemSizes an array containing the size of each item.
         * @param nItems the number of data elements.
         * @return true if the packet was sent successfully to all receivers.
         * @version 1.0
         */
        static bool CO_API send( const Connections& connections,
                                 Packet& packet, const void* const* items,
                                 const uint64_t* itemSizes,
                                 const size_t nItems );

        /** @internal Finish all pending send operations. */
        virtual void finish() { LBUNIMPLEMENTED; }
        //@}

        /**
         * The Notifier used by the ConnectionSet to detect readiness of a
         * Connection.
         */
#ifdef _WIN32
        typedef void* Notifier;
#else
        typedef int Notifier;
#endif
        /** @return the notifier signaling events on the connection. */
        virtual Notifier getNotifier() const = 0;

    protected:
        /** Construct a new connection. */
        Connection();

        /** Destruct this connection. */
        virtual ~Connection();

        /** @name Low-level IO methods. */
        //@{
        enum ReadStatus //!< error codes for readSync()
        {
            READ_TIMEOUT = -2,
            READ_ERROR   = -1
            // >= 0: nBytes read
        };

        /**
         * Start a read operation on the connection.
         *
         * This method is the low-level counterpart used by recvNB(),
         * implemented by the concrete connection.
         *
         * This function returns immediately. The operation's Notifier will
         * signal data availability, upon which readSync() is used to finish the
         * operation.
         *
         * @param buffer the buffer receiving the data.
         * @param bytes the number of bytes to read.
         * @sa readSync()
         */
        virtual void readNB( void* buffer, const uint64_t bytes ) = 0;

        /**
         * Finish reading data from the connection.
         *
         * This method is the low-level counterpart used by recvSync(),
         * implemented by the concrete connection. It may return with a partial
         * read.
         *
         * @param buffer the buffer receiving the data.
         * @param bytes the number of bytes to read.
         * @param block internal WAR parameter, ignore it in the implementation
         *              unless you know exactly why not.
         * @return the number of bytes read, or -1 upon error.
         */
        virtual int64_t readSync( void* buffer, const uint64_t bytes,
                                  const bool block ) = 0;

        /**
         * Write data to the connection.
         *
         * This method is the low-level counterpart used by send(), implemented
         * by the concrete connection. It may return with a partial write.
         *
         * @param buffer the buffer containing the message.
         * @param bytes the number of bytes to write.
         * @return the number of bytes written, or -1 upon error.
         */
        virtual int64_t write( const void* buffer, const uint64_t bytes ) = 0;
        //@}

        /** @internal @name State Changes */
        //@{
        /** @internal Set the connection parameter description. */
        CO_API void _setDescription( ConnectionDescriptionPtr description );

        CO_API void _setState( const State state ); //!< @internal

        /** @return the description for this connection. */
        CO_API ConnectionDescriptionPtr _getDescription();
        //@}

    private:
        detail::Connection* const _impl;
    };

    CO_API std::ostream& operator << ( std::ostream&, const Connection& );

#   include "connection.ipp" // template implementation

}
#endif //CO_CONNECTION_H