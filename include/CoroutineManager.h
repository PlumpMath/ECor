#pragma once

/**
 * Coroutine
 * @author xiao
 */

#include <stack>

#include <cstdint>

#include <ucontext.h>

#include "commondefs.h"

namespace ECor
{

struct CoroutineInfo
{
    long used;
    char* stack;
    ucontext_t context;
};


class CoroutineManager
{
    const size_t size;
    const size_t stackSize;
    size_t curNCoroutine;
    size_t lastNCoroutine;
    long clk;
    CoroutineInfo* cInfos;
    std::stack<size_t> freeCoroutineNumber;
    static void onCoroutineExit(CoroutineManager* manager); 
public:
    CoroutineManager(size_t _size, size_t _stackSize);
    ~CoroutineManager();

    void yield();
    void yield(size_t target);
    bool doWork(EPollServer* eServer, CoroutineCallback cCallback, ProcessCallback func, ConnectionInfo* info, uint32_t initCtlEvent);
    long getTimeUsed();

    static const size_t OUTPOOL = (size_t) -1;
};

}
