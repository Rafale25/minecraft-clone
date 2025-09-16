#include "profiler.hpp"
#include "LegitProfiler/ImGuiProfilerRenderer.h"
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <string>
#include <vector>

inline double nsToMs(int64_t ns) {
    return double(ns) / 1e6;
}

inline double nsToS(int64_t ns) {
    return double(ns) / 1e9;
}

namespace legit::Profiler
{
    static ImGuiUtils::ProfilersWindow _profiler_window;
    static std::vector<legit::ProfilerTask> _tasks_cpu;
    static std::vector<legit::ProfilerTask> _tasks_gpu;
    static legit::ProfilerTask _current_task;
    static double _frame_start_time = 0;
    static double _frame_start_time_gpu = 0;

    using namespace legit::Colors;
    static constexpr std::array<uint32_t, 5> _color_wheel = { turqoise, sunFlower, amethyst, emerald, pumpkin };
    static int _color_index = 0;
    static int _color_index_gpu = 0;

    struct _scopedTaskGPUInfo {
        std::string name;
        uint32_t color;
    };
    static std::vector<_scopedTaskGPUInfo> _gpuTasks; // temp data to store name/color info until querying times at end of frame

    // GLuint _query_object_start_time = 0;
    std::vector<GLuint> _query_objects;
    size_t _current_query_object = 0;

    GLuint getNewQueryObject()
    {
        if (_query_objects.size() >= _current_query_object) { // create new query object if not enough
            GLuint id = 0;
            glGenQueries(1, &id);
            _query_objects.push_back(id);
        }

        return _query_objects[_current_query_object++];
    }

// ----

    _ScopedTask::_ScopedTask(const std::string& name) {//, uint32_t color) {
        _current_task.startTime = glfwGetTime() - _frame_start_time;
        _current_task.name = name;
        _current_task.color = _color_wheel[_color_index];

        _color_index = (_color_index + 1) % _color_wheel.size();
    }

    _ScopedTask::~_ScopedTask() {
        _current_task.endTime = glfwGetTime() - _frame_start_time;
        _tasks_cpu.push_back(_current_task);
    }

    _ScopedTaskGPU::_ScopedTaskGPU(const std::string& name) {//, uint32_t color) {
        glQueryCounter(getNewQueryObject(), GL_TIMESTAMP);
        _gpuTasks.push_back({name, _color_wheel[_color_index]});
        _color_index = (_color_index + 1) % _color_wheel.size();
    }

    _ScopedTaskGPU::~_ScopedTaskGPU() {
        glQueryCounter(getNewQueryObject(), GL_TIMESTAMP);
    }

    void beginFrame() {
        _tasks_cpu.clear();
        _tasks_gpu.clear();
        _gpuTasks.clear();
        _color_index = 0;
        _color_index_gpu = 0;
        _current_query_object = 0;

        _frame_start_time = glfwGetTime();
        glQueryCounter(getNewQueryObject(), GL_TIMESTAMP); // _frame_start_time_gpu
    }

    void endFrame() {
        GLuint64 start_time{0};
        glGetQueryObjectui64v(_query_objects[0], GL_QUERY_RESULT, &start_time);

        // go over all scopedTaskGPU and get timestamps

        for (int i = 0 ; i < (int)_gpuTasks.size(); ++i) {
            const auto& [name, color] = _gpuTasks[i];

            GLuint64 t1{0}, t2{0};
            glGetQueryObjectui64v(_query_objects[1 + (i*2)+0], GL_QUERY_RESULT, &t1);
            glGetQueryObjectui64v(_query_objects[1 + (i*2)+1], GL_QUERY_RESULT, &t2);

            legit::ProfilerTask task {
                .startTime = nsToS(t1 - start_time),
                .endTime = nsToS(t2 - start_time),
                .name = name,
                .color = color
            };

            _tasks_gpu.push_back(task);
        }

        _profiler_window.loadFrameDataCPU(&_tasks_cpu[0], _tasks_cpu.size());
        _profiler_window.loadFrameDataGPU(&_tasks_gpu[0], _tasks_gpu.size());
        _profiler_window.Render();
    }

    _ScopedTask scopedTask(const std::string& name) {//, uint32_t color) {
        return _ScopedTask(name);//, color);
    }

    _ScopedTaskGPU scopedTaskGPU(const std::string& name) {//, uint32_t color) {
        return _ScopedTaskGPU(name);//, color);
    }
}
