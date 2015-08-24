#pragma once

/**
 * TCP Server socket, just support ipv4
 * @author xiao
 */

namespace ECor
{

class TCPSocket;
class InetAddress;
class EPollServer;

class TCPServerSocket
{
    friend class EPollServer;
    int fd;
    int err;
    int backlog;
public:

    TCPServerSocket();
    ~TCPServerSocket();

    bool bind(const InetAddress& address);

    /**
     * return the socket
     * @param socket
     * @return false if error occur, or true
     */
    bool accept(TCPSocket& socket);

    /**
     * this function must call before bind
     */
    void setBacklog(int _backlog);

    int getBacklog();

    int error();
};

}
