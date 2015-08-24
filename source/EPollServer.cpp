#include "EPollServer.h"

#include <errno.h>
#include <sys/epoll.h>

#include "TCPServerSocket.h"
#include "CoroutineManager.h"
#include "ETime.h"

namespace ECor
{

void processNewWork(EPollServer* eServer, ConnectionInfo * info, unsigned int initCtlEvent, unsigned int nCoroutine, ProcessCallback func);

EPollServer::EPollServer(CoroutineManager* _cManager)
    : err(0), ttfbTimeout(100), requestTimeout(500), cManager(_cManager)
{
}

EPollServer::~EPollServer()
{
}

bool EPollServer::init(int epollSize)
{
    fd = epoll_create(epollSize);
    if(fd == -1)
    {
        err = errno;
        return false;
    }
    return true;
}

bool EPollServer::start(TCPServerSocket& serverSocket, uint32_t initCtlEvent, size_t eventSize, ProcessCallback func)
{
    fd = epoll_create(eventSize);
    if(fd == -1)
    {
        err = errno;
        return false;
    }
    epoll_event sev;
    sev.events = EPOLLIN;
    sev.data.ptr = &serverSocket;
    if(epoll_ctl(fd, EPOLL_CTL_ADD, serverSocket.fd, &sev))
    {
        err = errno;
        return false;
    }
    epoll_event* cevs = new epoll_event[eventSize];
    initCtlEvent &= ~EPOLLONESHOT;
    initCtlEvent &= ~EPOLLET;
    initCtlEvent |= (EPOLLRDHUP | EPOLLERR | EPOLLHUP);
    while(true)
    {
        while(!waitQueue.empty() && cManager->doWork(this, processNewWork, func, waitQueue.front(), initCtlEvent))
            waitQueue.pop();
        int nfd = epoll_wait(fd, cevs, eventSize, EPOLLWAITTIMEOUT);
        if(nfd == -1 && errno == EINTR)
            continue;
        if(nfd == -1)
        {
            err = errno;
            delete[] cevs;
            return false;
        }
        for(int i = 0; i < nfd; ++i)
        {
            epoll_event& event = cevs[i];
            if(event.data.ptr == & serverSocket)
                processNewConnection(&serverSocket, initCtlEvent);
            else if(event.events & (EPOLLRDHUP | EPOLLERR | EPOLLHUP))
                processOldConnectionErr((ConnectionInfo*) event.data.ptr);
            else
                processOldConnection((ConnectionInfo*) event.data.ptr, func, initCtlEvent);
        }
        cleanBadSocket();
    }
    delete[] cevs;
    return true;
}

void EPollServer::setTTFBTimeout(unsigned int timeout)
{
    ttfbTimeout = timeout;
}

void EPollServer::setRequestTimeout(unsigned int timeout)
{
    requestTimeout = timeout;
}

void EPollServer::processNewConnection(TCPServerSocket* serverSocket, uint32_t initCtlEvent)
{
    ConnectionInfo* info = new ConnectionInfo;

    if(!serverSocket->accept(info->socket))
    {
        delete info;
        return;
    }

    info->socket.eServer = this;
    info->socket.cManager = cManager;
    info->socket.requestTimeout = requestTimeout;
    epoll_event ev;
    ev.events = initCtlEvent;
    ev.data.ptr = info;
    if(epoll_ctl(fd, EPOLL_CTL_ADD, info->socket.fd, &ev))
    {
        delete info;
        return;
    }
    info->activeClock = ETime::millisecond();
    info->nCoroutine = CoroutineManager::OUTPOOL;
    activeList.push_front(info);
    info->activeIterator = activeList.begin();
}

void EPollServer::processOldConnectionErr(ConnectionInfo* info)
{
    epoll_event ev;
    epoll_ctl(fd, EPOLL_CTL_DEL, info->socket.fd, &ev);
    info->socket.close();
    if(info->nCoroutine == CoroutineManager::OUTPOOL)
    {
        activeList.erase(info->activeIterator);
        delete info;
        return;
    }
    cManager->yield(info->nCoroutine);
}

void EPollServer::processOldConnection(ConnectionInfo* info, ProcessCallback func, uint32_t initCtlEvent)
{
    info->activeClock = ETime::millisecond();
    activeList.erase(info->activeIterator);
    activeList.push_front(info);
    info->activeIterator = activeList.begin();
    if(info->nCoroutine == CoroutineManager::OUTPOOL)
    {
        epoll_event ev;
        epoll_ctl(fd, EPOLL_CTL_DEL, info->socket.fd, &ev);
        waitQueue.push(info);
        if(cManager->doWork(this, processNewWork, func, waitQueue.front(), initCtlEvent))
            waitQueue.pop();
    }
    else
        cManager->yield(info->nCoroutine);
}

void EPollServer::ctlRecv(TCPSocket* socket)
{
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
    ev.data.ptr = transToConnectionInfoPtr(socket);
    if(epoll_ctl(fd, EPOLL_CTL_MOD, socket->fd, &ev) != 0)
        socket->close();
}

void EPollServer::ctlSend(TCPSocket* socket)
{
    epoll_event ev;
    ev.events = EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
    ev.data.ptr = transToConnectionInfoPtr(socket);
    if(epoll_ctl(fd, EPOLL_CTL_MOD, socket->fd, &ev) != 0)
        socket->close();
}

void EPollServer::processNewWork(EPollServer* eServer, ConnectionInfo * info, unsigned int initCtlEvent, unsigned int nCoroutine, ProcessCallback func)
{
    epoll_event ev;
    ev.events = 0;
    ev.data.ptr = info;
    int ret = epoll_ctl(eServer->fd, EPOLL_CTL_ADD, info->socket.fd, &ev);
    if(ret == -1)
    {
        eServer->activeList.erase(info->activeIterator);
        delete info;
        return;
    }
    info->nCoroutine = nCoroutine;
    bool ctin = func(&info->socket, info->socket.eServer, info->socket.cManager);
    if(!ctin)
    {
        eServer->activeList.erase(info->activeIterator);
        epoll_ctl(eServer->fd, EPOLL_CTL_DEL, info->socket.fd, &ev);
        delete info;
        return;
    }
    ev.events = initCtlEvent;
    ret = epoll_ctl(eServer->fd, EPOLL_CTL_MOD, info->socket.fd, &ev);
    if(ret == -1)
    {
        eServer->activeList.erase(info->activeIterator);
        epoll_ctl(eServer->fd, EPOLL_CTL_DEL, info->socket.fd, &ev);
        delete info;
        return;
    }
    info->socket.err = 0;
    info->socket.requestTimeout = eServer->requestTimeout;
    info->nCoroutine = CoroutineManager::OUTPOOL;
}

void EPollServer::cleanBadSocket()
{
    if(ttfbTimeout == 0)
        return;
    long clk = ETime::millisecond();
    while(!activeList.empty())
    {
        ConnectionInfo* info = activeList.back();
        if(clk - info->activeClock < ttfbTimeout)
            break;
        if(info->nCoroutine == CoroutineManager::OUTPOOL)
        {
            activeList.pop_back();
            epoll_event ev;
            epoll_ctl(fd, EPOLL_CTL_DEL, info->socket.fd, &ev);
            info->socket.close();
            delete info;
        }
        else
        {
            info->socket.close();
            cManager->yield(info->nCoroutine);
        }
    }
}

}
