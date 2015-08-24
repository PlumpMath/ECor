#include "CoroutineManager.h"

#include <cstdlib>
#include <cassert>

#include "ETime.h"

namespace ECor
{

void CoroutineManager::onCoroutineExit(CoroutineManager* _manager)
{
    CoroutineManager* manager = _manager;
    int ret = swapcontext(&manager->cInfos[0].context, &manager->cInfos[manager->curNCoroutine].context);
    assert(ret == 0);
    while(true)
    {
        manager->lastNCoroutine = manager->curNCoroutine;
        manager->curNCoroutine = 1;
        manager->freeCoroutineNumber.push(manager->lastNCoroutine);
        manager->cInfos[manager->lastNCoroutine].used = 0;
        ret = swapcontext(&manager->cInfos[0].context, &manager->cInfos[manager->curNCoroutine].context);
        assert(ret == 0);
    }
}

CoroutineManager::CoroutineManager(size_t _size, size_t _stackSize)
    : size(_size + 2), stackSize(_stackSize), curNCoroutine(1), lastNCoroutine(1)
{
    assert(size > 2);
    cInfos = new CoroutineInfo[size];
    for(size_t i = 0; i < size; ++i)
    {
        cInfos[i].used = 0;
        cInfos[i].stack = new char[stackSize];
    }
    int ret = getcontext(&cInfos[0].context);
    assert(ret == 0);
    cInfos[0].context.uc_link = NULL;
    cInfos[0].context.uc_stack.ss_sp = cInfos[0].stack;
    cInfos[0].context.uc_stack.ss_size = stackSize;
    typedef void (*_cbk)(void);
    makecontext(&cInfos[0].context, (_cbk) onCoroutineExit, 1, this);
    for(size_t i = 2; i < size; ++i)
        freeCoroutineNumber.push(i);
    ret = swapcontext(&cInfos[curNCoroutine].context, &cInfos[0].context);
    assert(ret == 0);
    clk = ETime::millisecond();
}

CoroutineManager::~CoroutineManager()
{
    for(size_t i = 0; i < size; ++i)
        delete[] cInfos[i].stack;
    delete[] cInfos;
}

void CoroutineManager::yield()
{
    size_t tmp = lastNCoroutine;
    lastNCoroutine = curNCoroutine;
    curNCoroutine = tmp;
    long cttmp = ETime::millisecond();
    cInfos[lastNCoroutine].used += (cttmp - clk);
    clk = cttmp;
#ifndef NDEBUG
    int ret = swapcontext(&cInfos[lastNCoroutine].context, &cInfos[curNCoroutine].context);
#else
    swapcontext(&cInfos[lastNCoroutine].context, &cInfos[curNCoroutine].context);
#endif
    assert(ret == 0);
}

void CoroutineManager::yield(size_t nCoroutine)
{
    lastNCoroutine = curNCoroutine;
    curNCoroutine = nCoroutine;
    long cttmp = ETime::millisecond();
    cInfos[lastNCoroutine].used += (cttmp - clk);
    clk = cttmp;
#ifndef NDEBUG
    int ret = swapcontext(&cInfos[lastNCoroutine].context, &cInfos[curNCoroutine].context);
#else
    swapcontext(&cInfos[lastNCoroutine].context, &cInfos[curNCoroutine].context);
#endif
    assert(ret == 0);
}

bool CoroutineManager::doWork(EPollServer* eServer, CoroutineCallback cCallback, ProcessCallback func, ConnectionInfo*info, uint32_t initCtlEvent)
{
    if(freeCoroutineNumber.empty())
        return false;

    size_t nc = freeCoroutineNumber.top();
    freeCoroutineNumber.pop();

    int ret = getcontext(&cInfos[nc].context);
    assert(ret == 0);
    cInfos[nc].context.uc_link = &cInfos[0].context;
    cInfos[nc].context.uc_stack.ss_sp = cInfos[nc].stack;
    cInfos[nc].context.uc_stack.ss_size = stackSize;
    typedef void (*_cbk) (void);
    makecontext(&cInfos[nc].context, (_cbk) cCallback, 5, eServer, info, (unsigned int)initCtlEvent, (unsigned int)nc, func);
    lastNCoroutine = curNCoroutine;
    curNCoroutine = nc;
    long cttmp = ETime::millisecond();
    cInfos[lastNCoroutine].used += (cttmp - clk);
    clk = cttmp;
    ret = swapcontext(&cInfos[lastNCoroutine].context, &cInfos[curNCoroutine].context);
    assert(ret == 0);
    return true;
}

long CoroutineManager::getTimeUsed()
{
    return cInfos[curNCoroutine].used + (ETime::millisecond() - clk);
}

}
