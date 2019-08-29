#include "ThreadPool.h"

ThreadPool::ThreadPool(int size) : halted(false)
{
    createThreadPool(size);
}

ThreadPool::~ThreadPool()
{
    if (!halted)
        Shutdown();
}

void ThreadPool::SubmitTask(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(task_queue_mtx);
        task_queue.push(task);
    }
    cond.notify_one();
}

void ThreadPool::Shutdown()
{
    halted = true;
    cond.notify_all();
    for (size_t i = 0; i < workers.size(); ++i)
    {
        workers[i].join();
    }
}

void ThreadPool::createThreadPool(int size)
{
    for (int i = 0; i < size; ++i)
    {
        workers.emplace_back([this] {
            for (;;)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(task_queue_mtx);
                    cond.wait(lock, [this] { return halted || !task_queue.empty(); });
                    if (halted && task_queue.empty())
                        break;
                    task = std::move(task_queue.front());
                    task_queue.pop();
                }
                task();
            }
        });
    }
}