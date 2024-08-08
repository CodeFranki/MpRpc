#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <thread>

// 异步写日志的队列
template <typename T>
class LockQueue
{
private:
    std::mutex m_mutex;
    std::queue<T> m_queue;
    std::condition_variable m_condvariable;

public:
    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_condvariable.notify_one(); // 因为只有一个线程进行写日志操作，如果有多线程需要向日志写东西，就需要通知所有的阻塞线程
    }
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            // 队列为空，线程进入阻塞状态
            m_condvariable.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }
};
