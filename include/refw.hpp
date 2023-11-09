
/*******************************************************************************
 * Author:          HRD29
 * Description:     Header file for framework
 * Date:            22.08.23
 * Reviewer:        ---
 * Versions:        1.0 
*******************************************************************************/
//#pragma once


#ifndef _HRD29_RequestEngineFW_
#define _HRD29_RequestEngineFW_

//#include <utility>//std::pair
//#include <cstring>//strcmp

#include "eventchannel.hpp"
#include "logger.hpp"
#include "factory.hpp"
#include "thread_pool.hpp"
#include "interface_thread_task.hpp"//ITask
#include "pnp.hpp"
#include "RequestSource.hpp"
#include "RETask.hpp"//maybe i need this
//#include <sys/eventfd.h> //eventfd
#include <sys/epoll.h>  //epoll
#include <cerrno>
//#include <unistd.h>



namespace hrd29
{

static bool disc_flag =  false;

template< typename KEY, typename ARGS>
class RequestEngineFW final //final- can not heritance
{
public:
    using Create_Func = std::function<std::shared_ptr<RETask>(ARGS)>;

    explicit RequestEngineFW(size_t thread_num_= CoresInCPU(), 
                            const std::string path_to_plugin_= "./plugins");//create factory
    RequestEngineFW(const RequestEngineFW& other_) = delete;
    RequestEngineFW& operator=(const RequestEngineFW& other_) = delete;
    ~RequestEngineFW() noexcept = default;//no impl

    void ConfigTask(const KEY& key, Create_Func func_);//facroty.add
    void ConfigRequestSource(std::shared_ptr<IRequestSource<KEY,ARGS>> request_source_);
    void Run();//listen epoll
    //if fd wake up
    //go to hasu- find the key,args
    //create factory
    //TP.add (the specific execute)
    //
    //void Stop(); //TODO: why no stop function ?you can implement with flag

private:
//watch-dog; todo at the end
    epoll_event* InitEpoll();
    void SetKAPair(const epoll_event& event, std::pair<KEY, ARGS>& key_args);
    void SetRETaskt(std::shared_ptr<RETask>& task_ptr, const std::pair<KEY, ARGS>& key_args);

    Factory<RETask, KEY , ARGS>* m_factory;
    ThreadPool m_thread_pool;
    DirMonitor m_dir_monitor;
    DllLoader m_plug_loader;

    std::unordered_map<int, std::shared_ptr<IRequestSource<KEY,ARGS>>> m_request_sources_;// impl in test
    int epoll_fd_m ; //epoll fd file(m_epoll_fd)
    //bool m_is_running;// should be atomic- do not use it but a global disc flag
    //void ConfigStopTask();
    Logger* m_logger;
};


/*******************************REFW IMP**********************************/


template<typename KEY, typename ARGS>
RequestEngineFW<KEY, ARGS>::RequestEngineFW(size_t thread_num_, const std::string path_to_plugin_) try : 
m_factory(Singleton<Factory<RETask, KEY, ARGS>>::GetInstance()),
m_thread_pool(thread_num_),
m_dir_monitor(path_to_plugin_),
m_plug_loader(m_dir_monitor.GetDispatcher()),
m_logger(Singleton<Logger>::GetInstance())
{
    //m_logger->SetLogLevel(Logger::Error);// move to cpp
    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "RequestEngineFW - Enter", true );

}

catch(const std::bad_alloc& ex) // Factory
{
    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DeviceSource::Read - EXIT", true );
    exit(EXIT_FAILURE);
}


/*OLD IMPL
template< typename KEY, typename ARGS>
RequestEngineFW<KEY, ARGS>::~RequestEngineFW() noexcept
{
    try
    {
        this->Stop();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    

}*/

template< typename KEY, typename ARGS>
void RequestEngineFW<KEY, ARGS>::ConfigTask(const KEY& key, Create_Func func_)
{
    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DeviceSource::ConfigTask - ENTER", true );
    m_factory->Add(key, func_);
    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DeviceSource::ConfigTask - EXIT", true );

}

template< typename KEY, typename ARGS>
void RequestEngineFW<KEY, ARGS>::ConfigRequestSource(std::shared_ptr<IRequestSource<KEY,ARGS>> request_source_)
{ 
    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DeviceSource::ConfigRequestSource - ENTER", true );
    m_request_sources_.emplace(request_source_->GetFd(), request_source_);
    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DeviceSource::ConfigRequestSource - EXIT", true );

}


template< typename KEY, typename ARGS>
void RequestEngineFW<KEY, ARGS>::Run()
{

//eopll, like in main_works

//listen epoll
    //if fd wake up
    //go to has table- find the key,args
    //create factory
    //TP.add (the specific execute)
    //


    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DeviceSource::Run - ENTER", true );
    const size_t num_sources = m_request_sources_.size();
    struct epoll_event* events_array = InitEpoll();
    
    int num_fds = 0;
    std::pair<KEY, ARGS> key_args;
    std::shared_ptr<RETask> task_ptr = nullptr;
    
    while(false == disc_flag)//flag for loop-  listen to fd in epoll- untill fd wakeup
    {

        //struct epoll_event event ;

        num_fds = epoll_wait(epoll_fd_m, events_array, num_sources, -1); //wait untill fd coming- an event happened
        if((-1 == num_fds) && (EINTR != errno))
        {
            m_logger->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "rewf- Failed epoll wait", true );
            return;
        }

        for(int i = 0; i < num_fds; ++i)
        {
            /*std::pair<KEY, ARGS> key_args =m_request_sources_.at(event[i].data.fd)->Read();
            task_ptr = m_factory->Create(key_args.first, key_args.second);
            */
            SetKAPair(events_array[i], key_args);
            SetRETaskt(task_ptr, key_args);
            m_thread_pool.AddTask(task_ptr);

        }   
        
    }
    
    delete[] events_array;

    task_ptr = DisconnectTask::CreateDefault();
    m_thread_pool.AddTask(task_ptr);
    if(-1 == close(epoll_fd_m))
    {
        m_logger->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "rewf- Failed close epoll", true );
        return;
    }
  
    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "rewf - Run-EXIT", true );
    
}

//TODO: NOT SURE THE STOP
/*template< typename KEY, typename ARGS>
void RequestEngineFW<KEY, ARGS>::Stop()
{
    
    m_is_running = false;
    //m_thread_pool.Stop();//no need the dtor of thread pool calls stop
    close(epoll_fd_m); //create event to stop the epoll
    
}*/


/************************Private Functions*************************/
template<typename KEY, typename ARGS>
epoll_event* RequestEngineFW<KEY, ARGS>::InitEpoll()
{
    //LOG_WRITE(m_logger, Logger::DEBUG, "InitEpoll - Enter", g_debug_print);

    const size_t num_of_sources = m_request_sources_.size();
    epoll_event* events_array = new epoll_event[num_of_sources];
    // 0 - nbd_event, 1 - stdin_event
    memset(events_array, 0, sizeof(epoll_event) * num_of_sources);

    epoll_fd_m= epoll_create1(EPOLL_CLOEXEC);
    if(-1 == epoll_fd_m)
    {
        exit(1);
    }

    auto it = m_request_sources_.begin();
    for(size_t i = 0; i < num_of_sources; ++i, ++it)
    {
        events_array[i].events = EPOLLIN;
        events_array[i].data.fd = it->first;
        if(-1 == epoll_ctl(epoll_fd_m, EPOLL_CTL_ADD, events_array[i].data.fd, events_array + i))
        {
            exit(1);
        }
    }

    //LOG_WRITE(m_logger, Logger::DEBUG, "InitEpoll - Exit", g_debug_print);

    return events_array;
}

template<typename KEY, typename ARGS>
void RequestEngineFW<KEY, ARGS>::SetKAPair(const epoll_event& event, std::pair<KEY, ARGS>& key_args)
{
    try
    {
        key_args = m_request_sources_.at(event.data.fd)->Read();
    }
    catch(const std::out_of_range& ex)
    {
        //LOG_WRITE(m_logger, Logger::ERROR, ex.what(), true);
        exit(EXIT_FAILURE);
    }
}

template<typename KEY, typename ARGS>
void RequestEngineFW<KEY, ARGS>::SetRETaskt(std::shared_ptr<RETask>& task_ptr, const std::pair<KEY, ARGS>& key_args)
{
    try
    {
        task_ptr = m_factory->Create(key_args.first, key_args.second);
    }
    catch(const std::out_of_range& ex)
    {
        //LOG_WRITE(m_logger, Logger::ERROR, ex.what(), true);
        exit(EXIT_FAILURE);
    }
}



}

#endif //_HRD29_RequestEngineFW_

























/*

run witout a make:
gb11+  driver_data.cpp drive_communicator.cpp eventchannel.hpp factory.hpp
interface_thread_task.hpp  logger.cpp myexception.hpp
pnp.cpp pq.hpp refw.cpp RequestSource.hpp RETask.hpp singleton.hpp thread_pool.cpp storage.cpp


*/