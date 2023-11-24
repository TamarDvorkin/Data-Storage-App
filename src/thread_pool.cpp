
#include <queue>
#include <iostream>
#include <exception>
#include<stdexcept>
#include <thread>
#include <vector>
#include "pq.hpp"
//#include "myexception.hpp"
#include "thread_pool.hpp"
#include "interface_thread_task.hpp"


namespace hrd29
{
/**************************************Task Derived impl***************************************/
    class TPTask : public ITask //suspned task
    {
    public:
        TPTask(std::mutex &mutex,std::condition_variable& cv, bool& suspend_) : ITask(ITask::Priority::ADMIN),m_mutex(mutex),m_cv(cv),
        m_is_suspend(suspend_)
        {}
        ~TPTask() override;
       

    private:
        void Execute() override;
        std::mutex &m_mutex;
        std::condition_variable& m_cv;
        bool& m_is_suspend;


    };

    class  STOPTASK : public ITask
    {
     public:
        STOPTASK():ITask(ITask::Priority::ADMIN)
        {}
        ~STOPTASK(){}
    private:
        void Execute() override
        {
                return;
        }

    };



    TPTask ::~TPTask(){};

    void TPTask::Execute() 
    {
       
        std::unique_lock<std::mutex>lock(m_mutex);
        //wait- dummy task just for waiting
        //other task should be :Write, Read, Trim, Flush
        m_cv.wait(lock,[this](){return !m_is_suspend;});
        
    } 


static void JoinThreads(std::vector<std::thread>& threads, const size_t& num_of_threads);
/*************************************************ThreadPool Imp*****************************************************/

    ThreadPool::ThreadPool(size_t thread_num):m_threads(thread_num),m_is_stop(false),
    m_is_suspend(false), m_suspend_thread(0),m_waiting_threads(0),m_active_threads(thread_num)
    {

        size_t i = 0;

        for (size_t i = 0; i < thread_num; ++i)
        {
            m_threads[i] = std::thread(&hrd29::ThreadPool::WorkThreadFunction,this);  
        }
        
    }

    ThreadPool::~ThreadPool()
    {
        Stop(); 

        for (int i = 0; i < m_threads.size(); ++i)
        {
            m_threads[i].join(); 
        }

    }

    void ThreadPool::AddTask(std::shared_ptr<ITask> task_)
    {
        m_wpq.Push(task_);
    }


    void ThreadPool::WakeUpThreads(size_t number)
    {

        size_t i=0;
      
        // i cant wake threads more then the orig size of the thread pool
        if(number > (m_threads.size() - m_active_threads))
        {
            for(;i<number;++i)
            {
                m_threads.emplace_back([this]{WorkThreadFunction(); });
            }
            
        }
        for(;i<number; ++i)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_cv.notify_one();
        }

    }

    void ThreadPool::Stop()
    {
        int i = 0;
        m_is_stop = true;
        std::shared_ptr<ITask> task(new STOPTASK());
  
        // i do not need lock because there is one in TPTask execute
        for(i=0; i<m_threads.size(); ++i)
        {
           
            m_wpq.Push(task);
        
        }
    }

    void ThreadPool::Suspend()
    {

        int i = 0;
        m_is_suspend = true;
        std::shared_ptr<ITask> task(new TPTask(m_mutex, m_cv, m_is_suspend));
        
        //wait untill all specified number  of threads  are suspended before proceeding  
       for(size_t i =0; i<m_active_threads; ++i) 
        {
            m_wpq.Push(task);
        }
  
    }

    void ThreadPool::Resume()
    {
        m_is_suspend = false;
        for(size_t i =0; i<m_active_threads;++i)
        {
            m_cv.notify_one();
        }
        
    }

    void ThreadPool::SetSize(size_t thread_num_)
    {
        //logic of setSize: you should suspend only 1 thread if
        //orig size is 4 and thread_num is 3
        

        //call suspend function
        int i =0;
        int delta = thread_num_ - m_active_threads;
        size_t vector_size = m_threads.size();

        if(true == m_is_suspend)
        {
            //check if user want more then size- add this logic to setSize
            if(delta>0)
            {
                //num of thread =  size
                // in loop add the delta from size to user want
            }
        }

        if(delta >0 &&(m_threads.size() == m_active_threads))//all threads active+ the user want more threads
        {
            for(int i=0; i<delta; ++i)
            {
                try
                {
                    m_threads.emplace_back([this]{WorkThreadFunction(); });
                }
                catch(const std::exception& e)
                {
                    std::cout <<"handle me latter-" <<e.what() << std::endl;;
                }

            }
        }

        else if(delta >0 &&(m_threads.size() > m_active_threads))
        {
            for(int i = 0; i<delta; ++i)
            {
                //wake up delta threads for the user to use
                std::lock_guard<std::mutex> lock(m_mutex);

                m_cv.notify_one();
            }
        }
        
        else if(delta<0)
        {
            for(size_t i=0; i<std::abs(delta);++i)
            {
                this->AddTask(std::make_shared<TPTask>(m_mutex, m_cv, m_is_suspend));//push those tasks

            }
        }

    //update activate suspned threads
    m_active_threads = thread_num_;
    m_suspend_thread = m_threads.size()- m_active_threads;
    
    }

    void ThreadPool::WorkThreadFunction()
    {
        while(0==m_is_stop)
        {
            auto task = m_wpq.Pop(); 
            
            task->Execute();
        } 
    }

/******************************************Free/static Functions*************************************************8*/

    size_t CoresInCPU()// for constructor to build num of threads
    {
        return std::max(std::thread::hardware_concurrency(), static_cast<unsigned int>(1));
    }

    static void JoinThreads(std::vector<std::thread>& threads, const size_t& num_of_threads)
    {
        for(size_t i = 0; i < num_of_threads; ++i)
        {
            threads[i].join();
        }

    }

}
