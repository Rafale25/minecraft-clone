#include "TaskQueue.hpp"

void TaskQueue::push_safe(std::function<void()> task) {
    const std::lock_guard<std::mutex> lock(m_taskQueueMutex);
    m_taskQueue.emplace_back(std::move(task));
};

void TaskQueue::execute() {
    // TODO: Don't understand why i can't pop an element from the task queue.
    // Using the auto for loop for the moment because it works.
    const std::lock_guard<std::mutex> lock(m_taskQueueMutex);
    for (auto &task: m_taskQueue) {
        task();
    }
    m_taskQueue.clear();
}

// std::function<void()> popFront() {
//     const std::lock_guard<std::mutex> lock(_task_queue_mutex);
//     // auto task = std::move(_task_queue.front());
//     auto task = _task_queue.front();
//     // _task_queue.pop();
//     _task_queue.pop_front();
//     return task;
// }

int32_t TaskQueue::count() const { return m_taskQueue.size(); };
