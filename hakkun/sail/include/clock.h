#pragma once

#include <chrono>

class Clock {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
    const char* mName = nullptr;

public:
    Clock(const char* name)
        : mStart(std::chrono::high_resolution_clock::now())
        , mName(name) { }

    void print() {
        auto now = std::chrono::high_resolution_clock::now();

        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - mStart).count();
        auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(now - mStart).count();

        printf("%s: %ldms/%ldµs\n", mName, elapsedMs, elapsedUs);
    }
};
