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
