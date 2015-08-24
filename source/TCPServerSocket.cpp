#include "TCPServerSocket.h"

#include <cassert>

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "InetAddress.h"
#include "TCPSocket.h"

namespace ECor
{

TCPServerSocket::TCPServerSocket()
    : backlog(64)
{
}

TCPServerSocket::~TCPServerSocket()
{
}
    
bool TCPServerSocket::bind(const InetAddress& address)
{
    sockaddr_in addr;
    assert(address.getType() == InetAddress::IPV4);
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd == -1)
    {
        err = errno;
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = address.getPort();
    addr.sin_addr.s_addr = address.getIp();
    int ret = ::bind(fd, (sockaddr*) &addr, sizeof(addr));
    if(ret == -1)
    {
        err = errno;
        return false;
    }
    ret = listen(fd, backlog);
    if(ret == -1)
    {
        err = errno;
        return false;
    }
    return true;
}

bool TCPServerSocket::accept(TCPSocket& socket)
{
    sockaddr_in addr;
    socklen_t addrSize = sizeof(addr);
    int cfd = ::accept(fd, (sockaddr*) &addr, &addrSize);
    if(cfd == -1)
    {
        err = errno;
        return false;
    }
    socket.fd = cfd;
    socket.err = 0;
    socket.remoteAddress = InetAddress(addr.sin_addr.s_addr, addr.sin_port);
    return true;
}

void TCPServerSocket::setBacklog(int _backlog)
{
    backlog = _backlog;
}

int TCPServerSocket::getBacklog()
{
    return backlog;
}

int TCPServerSocket::error()
{
    return err;
}

}
