ECor
====

**This is a EPoll Framework with Coroutine**

Getting start
-------------

###Echosvr###

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
    
    bool echoServer(ECor::TCPSocket* socket)
    {
        unsigned int size = 0;
        {
            char s[5];
            //socket->read(s, 4) return the number of bytes received, or -1 if an error occurred
            //if retuan val is not equals you expect(in this case is 4), it must be some error had occured
            //(eg:connection closed or timeout)
            //Just return and close the coroutine
            CHECKRS(socket->read(s, 4), 4, "read", socket->error(), false);
            s[4] = '\0';
            size = strtoul(s, NULL, 0);
        }
        char* str = new char[size + 1];
        CHECKRS(socket->read(str, size), size, "read", socket->error(), false);
        str[size] = '\0';
        printf("recv: %s\n", str);
        socket->write(str, size);
        return true;
    }
    
    int main(int argc, char** argv)
    {
        ECor::TCPServerSocket serverSocket;
        ECor::InetAddress addr(0, htons(10248));
        //Bind serverSocket to 0.0.0.0:10248
        CHECKRS(serverSocket.bind(addr), true, "bind", serverSocket.error(), 1);
        //Create a CoroutineManager with 4 worker coroutine which has 1024bytes stack
        ECor::CoroutineManager cManager(4, 1024);
        ECor::EPollServer eServer(&cManager);
        //Client socket will be closed if it do nothing in 5s
        eServer.setTTFBTimeout(5000);
        //Client socket will be closed if processing time consuming more than 2s
        eServer.setRequestTimeout(2000);
        //start the server;
        eServer.start(serverSocket, EPOLLIN, 10, echoServer);
        perror("");
        return 0;
    }
