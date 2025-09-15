#pragma once

#include <string>

#define ScopedTask(name) const auto _ = legit::Profiler::scopedTask(name)
#define ScopedTaskGPU(name) const auto _ = legit::Profiler::scopedTaskGPU(name)

namespace legit::Profiler {
    struct [[nodiscard]] _ScopedTask {
        _ScopedTask(const std::string& name);//, uint32_t color);
        ~_ScopedTask();
    };

    struct [[nodiscard]] _ScopedTaskGPU {
        _ScopedTaskGPU(const std::string& name);//, uint32_t color);
        ~_ScopedTaskGPU();
    };

    void beginFrame();
    void endFrame();
    _ScopedTask scopedTask(const std::string& name);//, uint32_t color = legit::Colors::turqoise);
    _ScopedTaskGPU scopedTaskGPU(const std::string& name);//, uint32_t color = legit::Colors::turqoise);
}
