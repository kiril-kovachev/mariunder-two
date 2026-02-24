#ifndef ASYNC_TIMER_H
#define ASYNC_TIMER_H

#include <Arduino.h>

// Non-blocking timer utility
class AsyncTimer {
public:
    AsyncTimer();
    AsyncTimer(uint32_t interval);

    void setInterval(uint32_t interval);
    void start();
    void restart();
    void stop();
    bool isExpired();
    bool isRunning();
    uint32_t getElapsed();
    uint32_t getRemaining();

private:
    uint32_t _interval;
    uint32_t _startTime;
    bool _running;
};

#endif // ASYNC_TIMER_H
