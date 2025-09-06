#pragma once

#include <cstdarg>
#include <cstdio>
#include <print>
#include <chrono>

#define _LOG_TIME_FORMAT "{:%H:%M:%S}"

#ifdef NO_CHRONO_CURRENT_ZONE
    #define _LOG_CHRONO 0
#else
    #define _LOG_CHRONO std::chrono::floor<std::chrono::milliseconds>(std::chrono::current_zone()->to_local(std::chrono::system_clock::now())) // TODO: current_zone()->to_local doesn't seem to work
#endif

#ifdef NO_CHRONO_CURRENT_ZONE
    #define _log(x, color, level, ...)  (std::print("\033[" color "m[" level "] " x "\033[0m\n", ##__VA_ARGS__))
#else
    #define _log(x, color, level, ...)  (std::print("\033[95m[" _LOG_TIME_FORMAT "]\033[0m " "\033[" color "m[" level "] " x "\033[0m\n", _LOG_CHRONO, ##__VA_ARGS__))
#endif


#define logT(x, ...) _log(x, "96", "Trace", ##__VA_ARGS__)
#define logD(x, ...) _log(x, "94", "Debug", ##__VA_ARGS__)
#define logI(x, ...) _log(x, "92", "Info", ##__VA_ARGS__)
#define logW(x, ...) _log(x, "93", "Warn", ##__VA_ARGS__)
#define logE(x, ...) _log(x, "91", "Error", ##__VA_ARGS__)
#define logF(x, ...) _log(x, "101", "Fatal", ##__VA_ARGS__)


// template <typename... Args>
// inline void my_print(std::format_string<Args...> fmt, Args&&... args) {
//     std::print(fmt, std::forward<Args>(args)...);
// }
