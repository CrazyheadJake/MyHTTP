#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class WorkerPool {
public:
    WorkerPool(size_t numThreads);
    void stop();
    void enqueue(std::function<void()> task);

private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_cv;
    bool m_stop = false;

    void run();
};