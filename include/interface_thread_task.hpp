/*******************************************************************************
 * Author: HRD29
 * Description: Header file for interface tread pool task for beeznas
 * Date: 017.08.2023
 * Reviewer: ---
 * Versions: 1.0 
*******************************************************************************/

#include "thread_pool.hpp"

#ifndef _HRD29_TASK_HPP_
#define _HRD29_TASK_HPP_

namespace hrd29
{
class ITask
{

public:

    enum Priority
    {
        LOW = 0,
        MEDIUM,
        HIGH,
        ADMIN = std::numeric_limits<int>::max() //standardized way to query various properties of arithmetic types
    };

    explicit ITask(Priority priority_ = MEDIUM);
    virtual ~ITask() noexcept = 0 ;
    bool operator<(const ITask& other_)//lesser my "lesser" compare
    {

        return m_priority <other_.m_priority;
    }
    

//children class tasks should be: Write, Read, Flush, Trim
private:
    friend class ThreadPool;
    virtual void Execute() = 0;

    Priority m_priority;
};


inline ITask::ITask(Priority priority_) : m_priority(priority_)
{}

inline ITask::~ITask()
{}



}//namespace hrd29

#endif /*_HRD29_TASK_HPP_*/
