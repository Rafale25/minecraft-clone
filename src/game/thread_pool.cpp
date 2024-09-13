#include "thread_pool.hpp"

#include <iostream>


ThreadPool::ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {
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

ThreadPool::~ThreadPool() {
    stop();
};

void ThreadPool::stop()
{
    {
        // Lock the queue to update the stop flag safely
        std::unique_lock<std::mutex> lock(_task_queue_mutex);
        _stop = true;
    }

    // Notify all threads
    _cv.notify_all();

    // Joining all worker threads to ensure they have completed their tasks
    for (auto& thread : _workers) {
        if (thread.joinable())
            thread.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(_task_queue_mutex);
        _task_queue.emplace(std::move(task));
    }
    _cv.notify_one();
}
