
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
    std::condition_variable m_cv;
};



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


template<typename T,typename Container,typename Compare>
T WPQ<T,Container,Compare>::Pop()
{
    T top;
    std::unique_lock<std::mutex> lock(m_mutex);
    //using uniqueu lock better for conditional variable:
   
    m_cv.wait(lock, [this](){return !m_wpq.empty();});
  
    try
    {
        if(!m_wpq.empty())
        {
            top = std::move(m_wpq.top());
            m_wpq.pop();

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
    
    return top;

}


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

