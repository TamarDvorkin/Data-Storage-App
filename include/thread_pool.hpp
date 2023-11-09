/*******************************************************************************
 * Author: Tamar
 * Description: Header file for thread pool
 * Date: 017.08.2023
 * Reviewer: ---
 * Versions: 1.0 
*******************************************************************************/
#ifndef _HRD29_THREAD_POOL_HPP_
#define _HRD29_THREAD_POOL_HPP_

#include <thread>
#include <memory>
#include <vector>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <future>   // std::promise, std::future

#include "singleton.hpp"
#include "interface_thread_task.hpp"
#include "pq.hpp"
#include "logger.hpp"

namespace hrd29
{
    size_t CoresInCPU();// for constructor to build num of threads
    bool SharedPtrITaskLAST(std::shared_ptr<ITask> x, std::shared_ptr<ITask> y);
    
    
    
    struct TaskCompare
    {
        bool operator()(const std::shared_ptr<ITask>&lhs, const std::shared_ptr<ITask>&rhs) const
        {
            return *lhs < *rhs;
        }
    };




class ThreadPool
{
public:

    explicit ThreadPool(size_t thread_num_ = CoresInCPU());
    //std::max - move to function.
    //If hardware_concurrency() returns 0, the number of thread will be 1
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    void AddTask(std::shared_ptr<ITask> task_); 
    //all tasks to preform must inherit from hrd29::ITask.

    void Stop();
    void Suspend();// if you call suspend you need to call resume and then setSize
    void Resume();
    void SetSize(size_t thread_num_);

private:

    void HowManyTaskToAdd(size_t number = 0);
    void WakeUpThreads(size_t number);
    void WorkThreadFunction();//thread function



    WPQ<std::shared_ptr<ITask>, std::vector<std::shared_ptr<ITask>>, TaskCompare> m_wpq;
    

    
    std::vector<std::thread> m_threads;
 
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::condition_variable m_suspend_cv;//new check- second cv for execute TPTASk
    std::atomic_bool m_is_stop;
    bool m_is_suspend;
    size_t m_suspend_thread;
    size_t m_active_threads;
    size_t m_waiting_threads;

};

}//namespace hrd29

#endif /*_HRD29_THREAD_POOL_HPP_*/




