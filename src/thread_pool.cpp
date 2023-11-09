
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
        //void Execute() override;//implement- not sure how

    private:
        void Execute() override;
        std::mutex &m_mutex;
        //std::condition_variable& m_cv;
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
                //throw MyException("exit thread\n");
                //m_is_stop  = true; no need- do this in stop
                return;
        }
        //bool& m_is_stop;


 
    };



    TPTask ::~TPTask(){};

    void TPTask::Execute() //not sure how to implement this
    {
       
        std::unique_lock<std::mutex>lock(m_mutex);
        //wait- dummy task just for waiting
        //other task should ne :Write, Read, Trim, Flush
        m_cv.wait(lock,[this](){return !m_is_suspend;});//its wait forever

        
    } //TODO: check if conditional in execute is different conditional variable
    //TODO: ypu changed the cv in Execute and int the construtor of TPTask


static void JoinThreads(std::vector<std::thread>& threads, const size_t& num_of_threads);
/*************************************************ThreadPool Imp*****************************************************/

    ThreadPool::ThreadPool(size_t thread_num):m_threads(thread_num),m_is_stop(false),
    m_is_suspend(false), m_suspend_thread(0),m_waiting_threads(0),m_active_threads(thread_num)
    {


        //logger1 = hrd29::Singleton<hrd29::Logger>::GetInstance();
        //logger1->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "inside ctor of ThreadPool", true );
        size_t i = 0;

        /*for(; i<thread_num; ++i)
        {
            m_threads.emplace_back([this]{WorkThreadFunction();});//check
        }*/

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

        
       //join threads inside the stop function
        //logger1->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "inside dtor of ThreadPool", true );
    }

    void ThreadPool::AddTask(std::shared_ptr<ITask> task_)
    {
        m_wpq.Push(task_);//check if error
    }


    void ThreadPool::WakeUpThreads(size_t number)
    {

        // i need to notify for number of threads to wake
        size_t i=0;
        //logger1->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "inside WakeUpThreads of ThreadPool", true );
        // i cant wake up more then the orig size
        if(number > (m_threads.size() - m_active_threads))
        {
            //throw std::runtime_error("WakeUpTread not enoch sleeping threads");
            for(;i<number;++i)
            {
                m_threads.emplace_back([this]{WorkThreadFunction(); });//dynamicly grow vector
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
        

        //this->SetSize(m_active_threads+m_suspend_thread);
    
        //add stop task
        //std::lock_guard<std::mutex>lock(m_mutex);
        // i do not need lock because there is one in TPTask execute
        for(i=0; i<m_threads.size(); ++i)
        {
            //this->AddTask(std::make_shared<TPTask>(m_mutex, m_cv, m_is_suspend));//push those tasks
        
            m_wpq.Push(task);
        
        }
        /*for(i=0; i<m_active_threads; ++i)
        {

            m_threads[i].join();//join for finished execute threads(was in the middle when stop called)

        }*/
        //JoinThreads(m_threads, m_active_threads);

    }

    void ThreadPool::Suspend()
    {

        int i = 0;
        m_is_suspend = true;
        std::shared_ptr<ITask> task(new TPTask(m_mutex, m_cv, m_is_suspend));
        //m_suspend_thread = m_active_threads;

        //this->SetSize(0);
        //wait untill all specified number  of threads  are suspended before proceeding  

       for(size_t i =0; i<m_active_threads; ++i) //m_cv.wait(lock,[this, thread_num_](){return m_suspend_thread == thread_num_;});
        {
            m_wpq.Push(task);
        }

        //m_active_threads=0;
        
    }

    void ThreadPool::Resume()
    {
        m_is_suspend = false;
        //this->SetSize(m_suspend_thread);// here i notify one *desired threads
        //m_suspend_thread = 0; //set number of therads on suspend to 0
        for(size_t i =0; i<m_active_threads;++i)
        {
            m_cv.notify_one();
        }
        

    }

    void ThreadPool::SetSize(size_t thread_num_)
    {
        //logic of setSize: you should suspend only 1 thread if
        //orig size is 4 and thread_num is 3
        //suspend only 1  

        //use suspend function
        int i =0;
        int delta = thread_num_ - m_active_threads;
        size_t vector_size = m_threads.size();

        if(true == m_is_suspend)
        {
            //check if user want more then size
            if(delta>0)
            {
                //DO NOT TOUCH THE SUSPEND! ONLY RESUME CHANGING THE SUSPENDS
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
                    //throw("ThreadPool SetSize function:  the emplace_back fail");
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

    void ThreadPool::WorkThreadFunction()//no cv here!
    {

        while(0==m_is_stop)//or while(1)
        {
            auto task = m_wpq.Pop(); //The auto keyword specifies that the type of the variable that is being declared will be automatically deducted
            
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
