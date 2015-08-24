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
class CoroutineManager;

typedef bool (*ProcessCallback)(TCPSocket*, EPollServer*,CoroutineManager*);

typedef void (*CoroutineCallback) (EPollServer* sServer, ConnectionInfo* info, unsigned int initCtlEvent, unsigned int nCoroutine, ProcessCallback func);

}
