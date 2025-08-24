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
    std::deque<std::function<void()>> _task_queue;
    std::mutex _task_queue_mutex;
};
