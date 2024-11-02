#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

#include <queue>
#include <vector>
#include <functional>

class TaskQueue {
public:
    void push_safe(std::function<void()> task) {
        const std::lock_guard<std::mutex> lock(_task_queue_mutex);
        _task_queue.push_back(std::move(task));
    };

    // std::function<void()> popFront() {
    //     const std::lock_guard<std::mutex> lock(_task_queue_mutex);
    //     // auto task = std::move(_task_queue.front());
    //     auto task = _task_queue.front();
    //     // _task_queue.pop();
    //     _task_queue.pop_front();
    //     return task;
    // }

    int count() const { return _task_queue.size(); };

public:
    // std::queue<std::function<void()>> _task_queue;
    std::deque<std::function<void()>> _task_queue;
    std::mutex _task_queue_mutex;
};

// https://www.geeksforgeeks.org/thread-pool-in-cpp/
class ThreadPool {
public:
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    ~ThreadPool();

    void stop();
    void enqueue(std::function<void()> task);

    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> _task_queue;
    std::mutex _task_queue_mutex;

private:
    std::condition_variable _cv;
    bool _stop = false;
};

/*
pool.enqueue([i] {
    cout << "Task " << i << " is running on thread "
            << this_thread::get_id() << endl;
});
*/
