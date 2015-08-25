#include <cstdio>

#include <cstdlib>

#include <arpa/inet.h>
#include <sys/epoll.h>

#include "CoroutineManager.h"
#include "EPollServer.h"
#include "TCPSocket.h"
#include "TCPServerSocket.h"
#include "InetAddress.h"

#define CHECKRS(RS, EXPECT, NAME, ERR, RET) if((RS) != (EXPECT)){ perror(NAME); return (RET); }

bool echoServer(ECor::TCPSocket* socket, ECor::EPollServer*, ECor::CoroutineManager*)
{
    std::string str;
    int ret = socket->readLine(str);
    if(ret < 0)
        perror("readline error");
    printf("recv: %s\n", str.c_str());
    socket->write(str.c_str(), str.size());
    return true;
}

int main(int argc, char** argv)
{
    ECor::TCPServerSocket serverSocket;
    ECor::InetAddress addr(0, htons(10248));
    CHECKRS(serverSocket.bind(addr), true, "bind", serverSocket.error(), 1);
    ECor::CoroutineManager cManager(4, 1024);
    ECor::EPollServer eServer(&cManager);
    eServer.setTTFBTimeout(5000);
    eServer.setRequestTimeout(2000);
    eServer.start(serverSocket, EPOLLIN, 10, echoServer);
    perror("");
    return 0;
}
