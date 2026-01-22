#pragma once

#include <chrono>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <vector>
#include <algorithm>
#include <iostream>

struct ScheduledTask {
    std::chrono::steady_clock::time_point time;
    std::function<void()> task;

    bool operator<(const ScheduledTask& other) const {
        return time > other.time;
    }
};

class Scheduler {
private:

    std::priority_queue<ScheduledTask> tasks;

    mutable std::mutex queue_mutex;
    std::condition_variable condition;

    std::atomic<bool> stop_flag = false;
    std::thread worker_thread;

    std::once_flag join_once_flag;

    void workerFunction();

public:
    Scheduler();
    ~Scheduler();

    void scheduleAfter(std::chrono::milliseconds delay, std::function<void()> task);

    void scheduleAt(std::chrono::steady_clock::time_point time, std::function<void()> task);

    void stop();
    void wait();
    
    bool empty() const;

    size_t size() const;

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;
};


