#pragma once

/**
 * Common type define
 * @author xiao
 */

#include <cstdint>

namespace ECor
{

class EPollServer;
class ConnectionInfo;
class TCPServerSocket;
class TCPSocket;

typedef bool (*ProcessCallback)(TCPSocket*);

typedef void (*CoroutineCallback) (EPollServer* sServer, ConnectionInfo* info, unsigned int initCtlEvent, unsigned int nCoroutine, ProcessCallback func);

}
