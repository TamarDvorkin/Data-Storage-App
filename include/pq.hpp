
/*******************************************************************************
 * Author: HRD29
 * Description: Header file for Waitable Priority Queue
 * Date: 16.08.2023
 * Reviewer: ---
 * Versions: 1.0 
*******************************************************************************/

#ifndef _HRD29_WAITABLE_PRIORITY_QUEUE_HPP_
#define _HRD29_WAITABLE_PRIORITY_QUEUE_HPP_

#include <queue>   
#include <mutex>
#include <condition_variable>
#include <iostream>


namespace hrd29
{

template<typename T,
        typename Container = std::vector<T>,
        typename Compare = std::less<typename Container::value_type>>//less means- from 10 to 1
class WPQ
{
public:

    WPQ(const Container& cont_  = Container(),const Compare& compare_ = Compare());
    WPQ(const WPQ& other_) = delete;
    WPQ& operator=(const WPQ& other_) = delete;
    ~WPQ() = default;

    void Push(const T& elem_);
    T Pop();//If the pq is empty, the thread will be in blocking state (mt).
    size_t Size() const;
    bool IsEmpty() const;
    void Clear();

private:
    std::priority_queue<T, Container, Compare> m_wpq;
    std::mutex m_mutex;
    //std::recursive_mutex m_mutex;
    //std::mutex m_mutex_shared; // Shared mutex for data access
    //std::mutex m_cvMutex; // Mutex for condition variable
    std::condition_variable m_cv;
};

/*
about recursive mutex:
in your pop you lock and then call is_empyu function
in is_empty function there is another mutex

tone thread get twice the mutex- THAT IS A RECURSIVE MUTEX


*/

template<typename T,typename Container,typename Compare>
WPQ<T,Container,Compare>::WPQ(const Container& cont_, const Compare& compare):m_wpq(compare,cont_)
{}

template<typename T,typename Container,typename Compare>
void WPQ<T,Container,Compare>::Push(const T& elem_)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_wpq.push(elem_);//m_wpq.push(std::move(elem));
    m_cv.notify_one();
}
/*why im using notify_one and not notify all*/
/*
If there are ten threads blocked on the condition variable, 
for example, notify_one() will unblock only one thread, 
while notify_all() will unblock them all. 

*/

/*
All threads will receive the signal from the condition variable when the main thread does notify_all(),
but what you are forgetting one crucial detail: after waking up after being notified by a condition variable,
the mutex gets automatically re-locked. That's how wait() works. 
and only one thread will be able to do that. 
All the other threads that wake up will also try to lock the mutex, 
but only the first one wins the race and will do that, 
and all the other threads will sleep and continue dreaming.

A thread after being notified will not return from wait() until that thread
successfully relocks the mutex, too.

To return from wait() two things must happen: the thread gets notified from the condition variable,
and the thread relocks the mutex, successfully. wait() unlocks the mutex and waits on the condition variable,
atomically, and relocks the mutex when it is notified.

https://stackoverflow.com/questions/57219650/stdcondition-variablenotify-all-only-wakes-up-one-thread-in-my-threadpool
*/

template<typename T,typename Container,typename Compare>
T WPQ<T,Container,Compare>::Pop()
{
    T top;
    std::unique_lock<std::mutex> lock(m_mutex);
    
    /*using uniqueu lock better for conditional variable:
    

    std::unique_lock allows  to explicitly unlock the mutex
    when you're waiting and then relock it when you wake up. 
    Important when dealing with condition variables to avoid potential deadlocks
    
    I changed it here in pop because i need to manually unlock mutex while waiting 
    for condition

    in other funciton- im using std::lock_guard and not uniqueue-
    i do not wait on a variable condition

    */


    m_cv.wait(lock, [this](){return !m_wpq.empty();});
    //if queuev empty throw - instwad of recursive mutex
    try
    {
        if(!m_wpq.empty())
        {
            top = std::move(m_wpq.top());// should i do move?
            m_wpq.pop();//try catch

        }

        else
        {
            // The queue is empty
            std::cout << "Cannot pop - queue is empty" << std::endl;

        }


    }

    catch(const std::exception& e)
    {
        std::cout << "Exception during pop: " << e.what() << std::endl;
    }
    
 

    //top = std::move(m_wpq.top());// should i do move?
    //m_wpq.pop();//try catch
    return top;

}

/*about choosing wait and not wait_for wait_untill*/
/*
wait- if ypu want o wait indefinitely untill an element is available
wait_for(lock, duration)- wait specific duration of time
wait_untill(lock, time_point) - wait untill specific point in time
*/

template<typename T,typename Container,typename Compare>
size_t WPQ<T,Container,Compare>::Size() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_wpq.size();
}

template<typename T,typename Container,typename Compare>
bool WPQ<T,Container,Compare>::IsEmpty() const //1 true empty 0 false not empty
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_wpq.empty();

}


/*my added clear function for the Stop function in the thread pool
clearing the queue before ending the scope of object

*/
template<typename T,typename Container,typename Compare>
void WPQ<T,Container,Compare>::Clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    while(!m_wpq.empty())
    {
        m_wpq.pop();//pop witout wait
    }
}


} // namespace hrd29

#endif // _HRD29_WAITABLE_PRIORITY_QUEUE_HPP_

