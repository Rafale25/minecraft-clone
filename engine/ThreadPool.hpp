#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>

// https://www.geeksforgeeks.org/thread-pool-in-cpp/
class ThreadPool {
public:
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    ~ThreadPool();

    void stop();
    void enqueue(std::function<void()> task);

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_taskQueue;
    std::mutex m_taskQueueMutex;

private:
    std::condition_variable m_cv;
    bool m_stop = false;
};

/*
pool.enqueue([i] {
    cout << "Task " << i << " is running on thread "
            << this_thread::get_id() << endl;
});
*/
