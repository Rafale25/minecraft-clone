#pragma once

// #include "LegitProfiler/ProfilerTask.h"
#include <string>
// #include <cstdint>

namespace legit::Profiler {
    struct [[nodiscard]] ScopedTask {
        ScopedTask(const std::string& name);//, uint32_t color);
        ~ScopedTask();
    };

    void beginFrame();
    void endFrame();
    ScopedTask scopedTask(const std::string& name);//, uint32_t color = legit::Colors::turqoise);
}
