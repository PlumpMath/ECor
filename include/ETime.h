#pragma once

#include <cstdlib>

#include <sys/time.h>

namespace ECor
{

class ETime
{
    long startTime;
public:
    ETime()
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        startTime = tv.tv_sec * 1000;
        startTime += tv.tv_usec / 1000;
    }

    long getMillisecond() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        long now;
        now = tv.tv_sec * 1000;
        now += tv.tv_usec / 1000;
        return now - startTime;
    }

    static const ETime etime;
    static long millisecond()
    {
        return etime.getMillisecond();
    }
};

}
