#include "ThreadPool.hpp"
#include "Logger.hpp"

ThreadPool::ThreadPool(size_t num_threads) {
    logI("[ThreadPool] thread count: {}", num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
        m_workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                // The reason for putting the below code here is to unlock the queue before
                // executing the task so that other threads can perform enqueue tasks
                {
                    // Locking the queue so that data
                    // can be shared safely
                    std::unique_lock<std::mutex> lock(m_taskQueueMutex);

                    // Waiting until there is a task to
                    // execute or the pool is stopped
                    m_cv.wait(lock, [this] {
                        return !m_taskQueue.empty() || m_stop;
                    });

                    // exit the thread in case the pool
                    // is stopped and there are no tasks
                    if (m_stop) { // && _task_queue.empty()) {
                        return;
                    }

                    // Get the next task from the queue
                    task = std::move(m_taskQueue.front());
                    m_taskQueue.pop();
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
        std::unique_lock<std::mutex> lock(m_taskQueueMutex);
        m_stop = true;
    }

    // Notify all threads
    m_cv.notify_all();

    // Joining all worker threads to ensure they have completed their tasks
    for (auto& thread : m_workers) {
        if (thread.joinable())
            thread.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(m_taskQueueMutex);
        m_taskQueue.emplace(std::move(task));
    }
    m_cv.notify_one();
}
