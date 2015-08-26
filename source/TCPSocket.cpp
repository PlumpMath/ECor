#include "TCPSocket.h"

#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

#include "EPollServer.h"
#include "CoroutineManager.h"
#include "ETime.h"

namespace ECor {

TCPSocket::TCPSocket()
    : fd(0), err(0), closed(false), requestTimeout(0), eServer(NULL), cManager(NULL)
{
}

TCPSocket::TCPSocket(int _fd, EPollServer* _eServer, CoroutineManager* _cManager, const InetAddress& _remoteAddress, long _requestTimeout)
    : fd(_fd), err(0), closed(false), requestTimeout(_requestTimeout), eServer(_eServer), cManager(_cManager), remoteAddress(_remoteAddress)
{
}

TCPSocket::~TCPSocket()
{
    close();
}

ssize_t TCPSocket::read(void* buffer, size_t size)
{
    size_t received = 0;
    eServer->ctlRecv(this);
    while(received < size)
    {
        if(requestTimeout != 0 && cManager->getTimeUsed() > requestTimeout)
            close();
        if(closed)
            return -2;
        ssize_t ret = recv(fd, ((char*) buffer) + received, size - received, MSG_DONTWAIT);
        if(ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            cManager->yield();
        else if(ret < 0 && errno == EINTR)
            continue;
        else if(ret < 0)
        {
            err = errno;
            return ret;
        }
        else if(ret == 0)
            return received;
        else
            received += ret;
    }
    return received;
}

ssize_t TCPSocket::write(const void* buffer, size_t size)
{
    size_t sent = 0;
    eServer->ctlSend(this);
    while(sent < size)
    {
        if(requestTimeout != 0 && cManager->getTimeUsed() > requestTimeout)
            close();
        if(closed)
            return -2;
        ssize_t ret = send(fd, ((const char*) buffer) + sent, size - sent, MSG_DONTWAIT);
        if(ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            cManager->yield();
        else if(ret < 0)
        {
            err = errno;
            return ret;
        }
        else if(ret == 0)
            return sent;
        else
        {
            sent += ret;
            cManager->yield();
        }
    }
    return sent;
}

ssize_t TCPSocket::readLine(std::string& str)
{
    size_t received = 0;
    eServer->ctlRecv(this);
    while(true)
    {
        if(requestTimeout != 0 && cManager->getTimeUsed() > requestTimeout)
            close();
        if(closed)
            return -2;
        char tmp[16];
        ssize_t ret = recv(fd, tmp, 16, MSG_DONTWAIT | MSG_PEEK);
        if(ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            cManager->yield();
            continue;
        }
        else if(ret < 0 && errno == EINTR)
            continue;
        else if(ret < 0)
        {
            err = errno;
            return ret;
        }
        else if(ret == 0)
            return received;
        for(int i = 0; i < ret; ++i)
        {
            if(tmp[i] == '\n')
            {
                int ret = this->read(tmp, i + 1);
                if(ret < 0 || ret != i + 1)
                    return -1;
                str.append(tmp, i);
                return received + i;
            }
        }
        int ret2 = this->read(tmp, ret);
        if(ret2 < 0)
            return -1;
        str.append(tmp, ret);
        received += ret;
    }
}

void TCPSocket::close()
{
    if(closed)
        return;
    closed = true;
    ::close(fd);
}

int TCPSocket::error()
{
    return err;
}

const InetAddress& TCPSocket::getRemoteAddress()
{
    return remoteAddress;
}

bool TCPSocket::isClosed()
{
    return closed;
}

void TCPSocket::setRequestTimeout(long timeout)
{
    requestTimeout = timeout;
}

long TCPSocket::getRequestTimeout()
{
    return requestTimeout;
}

}
