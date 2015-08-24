#pragma once

/**
 * Encapsulate the tcp socket
 * @author xiao
 */

#include <sys/types.h>

#include "InetAddress.h"
#include "commondefs.h"

namespace ECor
{

class TCPServerSocket;
class CoroutineManager;
class EPollServer;
class ConnectionInfo;

class TCPSocket
{
    int fd;
    int err;
    bool closed;
    long requestTimeout;
    EPollServer* eServer;
    CoroutineManager* cManager;
    InetAddress remoteAddress;

private:
    friend class EPollServer;
    friend class CoroutineManager;
    friend class TCPServerSocket;
    friend class ConnectionInfo;

    TCPSocket();

    TCPSocket(int _fd, EPollServer* _eServer, CoroutineManager* _cManager, const InetAddress& _remoteAddress, long _requestTimeout);

public:
    ~TCPSocket();

    /**
     * @param buffer
     * @param size the length you expect to read
     * @return These calls return the number of bytes received, or -1 if an error occurred, or -2 if closed.
     *      The return value will be 0 when the peer has performed an orderly shutdown.
     *      If the return value less then size, it means the socket has been closed.
     *      In other word, the return value should be equal to the size, except some error occur.
     */
    ssize_t read(void* buffer, size_t size);

    /**
     * @param buffer
     * @param size the length you expect to write
     * @return These calls return the number of bytes sent, or -1 if an error occurred, or -2 if closed.
     *      The return value will be 0 when the peer has performed an orderly shutdown.
     *      If the return value less then size, it means the socket has been closed.
     *      In other word, the return value should be equal to the size, except some error occur.
     */
    ssize_t write(const void* buffer, size_t size);

    /**
     * close the socket, and remove from EPollServer.
     */
    void close();

    /**
     * return the last error
     * @return error
     */
    int error();

    bool isClosed();

    void setRequestTimeout(clock_t timeout);

    clock_t getRequestTimeout();

    const InetAddress& getRemoteAddress();

};

}
