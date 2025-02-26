#pragma once

#include <iostream>
#include <format>
#include <unordered_map>
#include <chrono>

struct Chrono {

    Chrono() {
        t1 = std::chrono::high_resolution_clock::now();
    }

    double getTimeMs() {
        auto t2 = std::chrono::high_resolution_clock::now();
        auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
        std::chrono::duration<double, std::milli> ms_double = t2 - t1;
        return ms_double.count();
    }

    void log() {
        printf("%fms\n", getTimeMs());
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> t1;
};

struct Timing {
    int32_t count = 0;
    // bool started = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> t;
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

        _timings[name].t = std::chrono::high_resolution_clock::now();
    }

    void stop(const std::string& name, bool print = false) {
        auto t = std::chrono::high_resolution_clock::now();

        auto& timing = _timings.at(name);

        std::chrono::duration<double, std::milli> duration = t - timing.t;
        // timing.current_average = (timing.current_average * timing.count + duration.count()) / (timing.count + 1);
        timing.current_average = duration.count();

        if (print) {
            printf("Timing: %s -- %f ms\n", name.c_str(), duration.count());
        }

        timing.count += 1;
    }

    std::string dump() const {
        std::string result;

        for (const auto& [name, timing] : _timings) {
            result += std::format("{}: {:.4f}ms \n", name, timing.current_average);
        }

        return result;
    }

private:
    std::unordered_map<std::string, Timing> _timings;
};
