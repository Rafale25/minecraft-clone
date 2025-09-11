#include "profiler.hpp"
#include "LegitProfiler/ImGuiProfilerRenderer.h"
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

namespace legit::Profiler
{
    static ImGuiUtils::ProfilersWindow _profiler_window;
    static std::vector<legit::ProfilerTask> _tasks;
    static legit::ProfilerTask _current_task;
    static double _frame_start_time = 0;

    using namespace legit::Colors;
    static constexpr std::array<uint32_t, 5> _color_wheel = { turqoise, sunFlower, amethyst, emerald, pumpkin };
    static int _color_index = 0;

    ScopedTask::ScopedTask(const std::string& name, uint32_t color) {
        _current_task.startTime = glfwGetTime() - _frame_start_time;
        _current_task.name = name;
        // _current_task.color = color;
        _current_task.color = _color_wheel[_color_index];

        _color_index = (_color_index + 1) % _color_wheel.size();
    }

    ScopedTask::~ScopedTask() {
        _current_task.endTime = glfwGetTime() - _frame_start_time;
        _tasks.push_back(_current_task);
    }

    void beginFrame() {
        _frame_start_time = glfwGetTime();
    }

    void endFrame() {
        _profiler_window.cpuGraph.LoadFrameData(&_tasks[0], _tasks.size());
        _profiler_window.Render();
        _tasks.clear();
    }

    ScopedTask scopedTask(const std::string& name, uint32_t color) {
        return ScopedTask(name, color);
    }
}
