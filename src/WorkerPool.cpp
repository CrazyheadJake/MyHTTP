#include "WorkerPool.h"
#include <iostream>

WorkerPool::WorkerPool(size_t numThreads)
{
    for (size_t i = 0; i < numThreads; ++i) {
        m_workers.emplace_back([this] { this->run(); });
    }
}

WorkerPool::~WorkerPool()
{
    m_queueMutex.lock();
    m_stop = true;
    m_queueMutex.unlock();
    m_cv.notify_all();
    for (std::thread &worker : m_workers) {
        if (worker.joinable())
            worker.join();
    }
}

void WorkerPool::enqueue(std::function<void()> task)
{
    m_queueMutex.lock();
    m_tasks.push(std::move(task));
    m_queueMutex.unlock();
    m_cv.notify_one();
}

void WorkerPool::run()
{
    std::function<void()> task;
    while (true) {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            // Wait until we are notified and there is a task or we are stopping
            m_cv.wait(lock, [&]{ return m_stop || !m_tasks.empty(); });
            std::cout << "Worker thread woke up: \n" << std::this_thread::get_id() << "\n";
            std::cout << "Number of times woken up total: " << ++m_count << "\n";
            // Handle shutdown
            if (m_stop && m_tasks.empty()) return;

            task = std::move(m_tasks.front());
            m_tasks.pop();
        } // unlock before running the task
        task(); // run the job
    }
}