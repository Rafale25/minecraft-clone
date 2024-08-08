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
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {
        std::cout << "num_threads: " << num_threads << std::endl;
        for (size_t i = 0; i < num_threads; ++i) {
            _workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    // The reason for putting the below code here is to unlock the queue before
                    // executing the task so that other threads can perform enqueue tasks
                    {
                        // Locking the queue so that data
                        // can be shared safely
                        std::unique_lock<std::mutex> lock(_task_queue_mutex);

                        // Waiting until there is a task to
                        // execute or the pool is stopped
                        _cv.wait(lock, [this] {
                            return !_task_queue.empty() || _stop;
                        });

                        // exit the thread in case the pool
                        // is stopped and there are no tasks
                        if (_stop && _task_queue.empty()) {
                            return;
                        }

                        // Get the next task from the queue
                        task = std::move(_task_queue.front());
                        _task_queue.pop();
                    }

                    task();
                }
            });
        }
    };

    ~ThreadPool() {
        {
            // Lock the queue to update the stop flag safely
            std::unique_lock<std::mutex> lock(_task_queue_mutex);
            _stop = true;
        }

        // Notify all threads
        _cv.notify_all();

        // Joining all worker threads to ensure they have completed their tasks
        for (auto& thread : _workers) {
            thread.join();
        }
    };

    void enqueue(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(_task_queue_mutex);
            _task_queue.emplace(std::move(task));
        }
        _cv.notify_one();
    }

    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> _task_queue;
    std::mutex _task_queue_mutex;

    std::condition_variable _cv;
    bool _stop = false;
};

/*
pool.enqueue([i] {
    cout << "Task " << i << " is running on thread "
            << this_thread::get_id() << endl;
});
*/
