#pragma once

#include <iostream>
#include <chrono>

using namespace std::chrono;

struct Chrono {

    Chrono() {
        t1 = high_resolution_clock::now();
    }

    double getTimeMs() {
        auto t2 = high_resolution_clock::now();
        // auto ms_int = duration_cast<milliseconds>(t2 - t1);
        duration<double, std::milli> ms_double = t2 - t1;
        return ms_double.count();
    }

    void log() {
        printf("%fms\n", getTimeMs());
    }

    system_clock::time_point t1;
};
