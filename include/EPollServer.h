#pragma once

/**
 * EPollManager
 * @author xiao
 */

#include <queue>
#include <list>

#include "TCPSocket.h"
#include "TCPServerSocket.h"
#include "commondefs.h"

namespace ECor
{

struct ConnectionInfo
{
    clock_t activeClock;
    size_t nCoroutine;
    TCPSocket socket;
    std::list<ConnectionInfo*>::iterator activeIterator;
};

class EPollServer
{
    int fd;
    int err;
    unsigned int ttfbTimeout;
    unsigned int requestTimeout;
    CoroutineManager* cManager;
    std::queue<ConnectionInfo*> waitQueue;
    std::list<ConnectionInfo*> activeList;
public:
    EPollServer(CoroutineManager* _cManager);
    ~EPollServer();

    bool init(int epollSize);

    bool start(TCPServerSocket& serverSocket, uint32_t initCtlEvent, size_t eventSize, ProcessCallback func);

    //in millisecond
    void setTTFBTimeout(unsigned int timeout);

    //in millisecond
    void setRequestTimeout(unsigned int timeout);

    int error();

    void ctlSend(TCPSocket* socket);
    void ctlRecv(TCPSocket* socket);

private:
    void processNewConnection(TCPServerSocket* serverSocket, uint32_t initCtlEvent);
    void processOldConnectionErr(ConnectionInfo* info);
    void processOldConnection(ConnectionInfo* info, ProcessCallback func, uint32_t initCtlEvent);

    void cleanBadSocket();

    static const int EPOLLWAITTIMEOUT = 1;

    static void processNewWork(EPollServer* eServer, ConnectionInfo* info, unsigned int initCtlEvent, unsigned int nCoroutine, ProcessCallback func);

private:
    static ConnectionInfo* transToConnectionInfoPtr(TCPSocket* socket)
    {
        return (ConnectionInfo*) ((char*)socket - offsetof(ConnectionInfo, socket));
    }
};

}
