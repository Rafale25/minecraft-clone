#pragma once

#include <thread>
#include <mutex>

#include <queue>
#include <vector>
#include <functional>


class TaskQueue {
public:
    void push(std::function<void()> task);
    std::function<void()> front();
    int count() const { return task_queue.size(); };

private:
    std::queue<std::function<void()>> task_queue;

    std::vector<std::thread> workers;
};


// class ThreadPool {

// };
