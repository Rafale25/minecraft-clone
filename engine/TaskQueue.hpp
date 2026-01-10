#pragma once

#include <mutex>
#include <deque>
#include <functional>

class TaskQueue {
public:
    void push_safe(std::function<void()> task);
    void execute();
    int32_t count() const;

public:
    std::deque<std::function<void()>> m_taskQueue;
    std::mutex m_taskQueueMutex;
};
