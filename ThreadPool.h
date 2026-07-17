#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

// Fixed-size pool of worker threads pulling tasks off a shared queue.
//
// Why a condition_variable instead of a worker thread just spinning on
// "while(queue.empty()) {}"? Spinning burns 100% CPU on every idle worker
// thread even when there's nothing to do. A condition_variable lets idle
// threads sleep (block, using ~0 CPU) until notified that work exists.
class ThreadPool
{
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> stopping{false};

    void workerLoop()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queueMutex);

                // Sleep here until either a task arrives or we're told to stop.
                // Without this predicate-checking wait, a thread could wake up,
                // find the queue still empty (a "spurious wakeup"), and crash
                // trying to pop from it — the predicate guards against that.
                cv.wait(lock, [this] { return stopping || !tasks.empty(); });

                if (stopping && tasks.empty())
                    return; // drain remaining tasks before actually exiting

                if (tasks.empty())
                    continue;

                task = std::move(tasks.front());
                tasks.pop();
            }

            task(); // run outside the lock so other workers aren't blocked by it
        }
    }

public:
    explicit ThreadPool(size_t numThreads)
    {
        for (size_t i = 0; i < numThreads; ++i)
            workers.emplace_back(&ThreadPool::workerLoop, this);
    }

    void submit(std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            tasks.push(std::move(task));
        }
        cv.notify_one(); // wake exactly one sleeping worker to pick this up
    }

    // Signals all workers to finish remaining tasks then exit, and joins them.
    ~ThreadPool()
    {
        stopping = true;
        cv.notify_all(); // wake ALL sleeping workers so they can see the flag
        for (auto& w : workers)
            if (w.joinable())
                w.join();
    }

    size_t size() const { return workers.size(); }
};

#endif
