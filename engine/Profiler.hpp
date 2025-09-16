#pragma once

#include <string>

#define ScopedTask(name) const auto _scoped_task_cpu = legit::Profiler::_ScopedTask(name)
#define ScopedTaskGPU(name) const auto _scoped_task_gpu = legit::Profiler::_ScopedTaskGPU(name)

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
}
