#pragma once

#include <queue>
#include <thread>
#include <mutex>    //  pthread_mutex_t
#include <condition_variable>   //pthread_condition_t

//  异步写日志的消息队列

template<typename T>
class MsgQueue
{
public:
    // 多个worker线程都会写日志queue（epoll中）
    void Push(const T& msg)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(msg);
        m_condvar.notify_all(); //也可用notify_one
    }
    // 一个线程读日志，写入文件
    void Pop(T& data)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_queue.empty())
        {
            // msg队列为空，线程进入wait状态,并将锁释放掉
            m_condvar.wait(lock);
        }
        data =  m_queue.front();
        m_queue.pop();
        return;
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvar;
};
