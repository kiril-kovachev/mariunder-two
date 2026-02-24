#include "AsyncTimer.h"

AsyncTimer::AsyncTimer() : _interval(0), _startTime(0), _running(false) {}

AsyncTimer::AsyncTimer(uint32_t interval) : _interval(interval), _startTime(0), _running(false) {}

void AsyncTimer::setInterval(uint32_t interval) {
    _interval = interval;
}

void AsyncTimer::start() {
    _startTime = millis();
    _running = true;
}

void AsyncTimer::restart() {
    start();
}

void AsyncTimer::stop() {
    _running = false;
}

bool AsyncTimer::isExpired() {
    if (!_running) return false;
    return (millis() - _startTime) >= _interval;
}

bool AsyncTimer::isRunning() {
    return _running;
}

uint32_t AsyncTimer::getElapsed() {
    if (!_running) return 0;
    return millis() - _startTime;
}

uint32_t AsyncTimer::getRemaining() {
    if (!_running) return 0;
    uint32_t elapsed = getElapsed();
    if (elapsed >= _interval) return 0;
    return _interval - elapsed;
}
