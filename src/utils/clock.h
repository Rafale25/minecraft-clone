#pragma once

#include <iostream>
#include <format>
#include <unordered_map>
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

struct Timing {
    int count = 0;
    // bool started = false;
    system_clock::time_point t;
    double current_average = 0.0;
    // (currentAverage * currentNumberOfItems + X) / (currentNumberOfItems + 1)
};

#define SINGLETON_NO_CONTRUCTORS(T) \
    T(const T&) = delete;\
    T& operator=(const T&) = delete;\
    T(T&&) = delete;\
    T& operator=(T&&) = delete;\

class SimpleProfiler {
public:
    SimpleProfiler() = default;
    ~SimpleProfiler() = default;

    SINGLETON_NO_CONTRUCTORS(SimpleProfiler)

    static SimpleProfiler& instance() {
        static SimpleProfiler instance;
        return instance;
    }

    void start(const std::string& name) {
        // TODO: use below code to make Profile works in multithreaded environment
        // std::thread::id this_id = std::this_thread::get_id();

        auto &timing = _timings[name];
        timing.t = high_resolution_clock::now();
    }

    void stop(const std::string& name) {
        auto t = high_resolution_clock::now();

        auto& timing = _timings.at(name);

        auto duration = t - timing.t;
        timing.current_average = (timing.current_average * timing.count + duration.count()) / (timing.count + 1);
        timing.count += 1;
    }

    std::string dump() const {
        std::string result;

        for (const auto& [name, timing] : _timings) {
            result += std::format("{}: {}ns \n", name, timing.current_average);
        }

        return result;
    }

private:
    std::unordered_map<std::string, Timing> _timings;
};
