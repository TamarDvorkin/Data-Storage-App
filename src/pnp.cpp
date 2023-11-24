#include <string>   //string
#include <fstream>  //ofstream
#include <mutex>  //mutex
#include <memory>
#include <atomic>
#include <iostream>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/inotify.h>
#include <sys/eventfd.h> //eventfd
#include <sys/epoll.h>  //epoll

#include "singleton.hpp"
#include "pnp.hpp"
#include "logger.hpp"

namespace hrd29
{

/*********************************DirMonitor Imp************************************/
    DirMonitor::DirMonitor(std::string dir_path_):m_dir_path(dir_path_),m_dispatcher() ,listen(true)
    { 
        logger1= Singleton<Logger>::GetInstance();
        logger1->SetLogLevel(Logger::Error);

        event_fd = eventfd(0,0);
        if(-1 == event_fd)
        {
            logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "pnp DirMonitor- Failed to create eventfd", true );

        }

        fd = inotify_init1(IN_NONBLOCK);
        if(-1 == fd)
        {
            
            logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "pnp DirMonitor- Failed inotify_init1", true );
        }
        //wd add to watch
        wd = inotify_add_watch(fd, dir_path_.c_str(),IN_ALL_EVENTS);
        
        
        if(-1 == wd)
        {
            
            logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "pnp DirMonitor- Failed inotify_add_watch", true );
        }

        m_thread = std::thread([this]{ListenForEventTheard();});
        

    }


    DirMonitor::~DirMonitor()
    {
        listen = false;
        inotify_rm_watch(fd,wd);
        m_thread.join();

    }

    void DirMonitor::Add(std::string path_to_so_)
    {
        m_dispatcher.NotifyAll(path_to_so_);
    }

    Dispatcher<std::string>* DirMonitor::GetDispatcher()
    {
        return &m_dispatcher;

    }

    //thread function
    void DirMonitor::ListenForEventTheard()
    {
        char buffer[4096]; //4096- common page size- store events
        __attribute__((aligned(__alignof__(struct inotify_event))));
        struct inotify_event* in_event; 
        int max_events = 2;
        struct epoll_event events[2];
        ssize_t len = 0;
        ssize_t read_result;
        uint64_t event;

        size_t  i =0;
        size_t j =0;
        int event_count;
        int triggered_fd;
        int epoll_fd = epoll_create(1); // should be more then 0!
        
        //create epoll instance
        if(-1 == epoll_fd)
        {
            
            logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "pnp DirMonitor- Failed epoll_create", true );

            return;
        }
        //adding event_fd and inotify_fd to epoll
        events[0].data.fd = event_fd;//event_fd (the non-blocking wake-up event)
        events[0].events = EPOLLIN; //setting the events field for the event_fd entry in the array
        //EPOLLIN- reading data from event_fd, you want to be notified when the event_fd is ready for reading
        
        events[1].data.fd = fd;
        events[1].events = EPOLLIN;
        
        //use ctl -control an epoll file descriptor.
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event_fd, &events[0]);
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &events[1]);

        while(listen)
        {
           
            // using epoll for timeout for stopping thread function(stop read)
            // the distractor would chanage the flag to false
            // and the when calliung again the thread function the thread
            // will not get into the loop(flag is false)
         
        
            event_count = epoll_wait(epoll_fd, events,max_events, -1);
            //wait for max 2 event, -1 means- untill some event happends
            if(-1 == event_count)
            {
               
                logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "pnp DirMonitor- Failed epoll_wait", true);

                break;
            }

            for(i=0; i<event_count; ++i)
            {
                triggered_fd = events[i].data.fd;
                if(triggered_fd == event_fd)
                {
                    
                    logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "pnp DirMonitor- Failed triggered_fd", true);

                    return;

                }
            
                else if(this->fd == triggered_fd)
                {
                    len = read(this->fd, buffer, sizeof(buffer));
                    if(len<0)
                    {
                         
                        logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "pnp DirMonitor- Failed read", true);

                    }

                    while(j<len)
                    {
                        in_event = reinterpret_cast<struct inotify_event*>(&buffer[j]);
                        if(in_event->mask == IN_MOVED_TO || in_event->mask == IN_MODIFY)
                        {
                            std::cout <<"in mask even"<< std::endl;
                            std::string path_to_so = m_dir_path + "/" + in_event->name;
                            Add(path_to_so); // notify the file path to listeners
                        }
                        j+= sizeof(struct inotify_event)+ in_event->len; 

                        std::cout <<"in mask even outside if"<< std::endl;  
                    }
                
                }
        
            }
        }
        std::cout <<"in end of thread func"<< std::endl;
    }


/**************************DllLoader Imp*******************************/

DllLoader::DllLoader(Dispatcher<std::string>* dis_):m_callback(dis_, *this, &DllLoader::LoadPlugin)
{
    logger2= Singleton<Logger>::GetInstance();
    logger2->SetLogLevel(Logger::Error);
    std::cout<<"end of DllLoader constructor"<< std::endl;
}

DllLoader::~DllLoader()
{
    void* dir_to_handle;
    for(auto& dir_pair : m_loaded_directories)
    {
        dir_to_handle = dir_pair.second;
        if(dir_to_handle != nullptr)
        {
            if(0!= dlclose(dir_to_handle))
            {
                logger2->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "pnp DllLoader- Failed dlclose", true);
            }
        }
    }

    std::cout<<"end of DllLoader dtor"<< std::endl;
}

void DllLoader::LoadPlugin(const std::string& path_to_so_)
{
    void* dir_to_handle;
    std::cout<<"start of DllLoader LoadPlugIn"<< std::endl;
    dir_to_handle= dlopen(path_to_so_.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if(nullptr == dir_to_handle)
    {
         
        logger2->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "pnp DllLoader- Failed dlopen", true);
        std::cerr << dlerror() << std::endl;

        exit(EXIT_FAILURE);
    }
    // push the handles in the vector
    m_loaded_directories.push_back(std::make_pair(path_to_so_, dir_to_handle));
    std::cout<< "Loaded dir from: " <<path_to_so_ << std::endl;
    std::cout<<"end of DllLoader LoadPlugIn"<< std::endl;

}

}























//how to create s.o (shared object) file:
//gd11+ -shared -fpic test/pnp_so_file.cpp -o plg.so

//how to run this program
//in 1 terminal:
//gb11+ -rdynamic pnp.cpp logger.cpp  pnp_test.cpp -o "pnp.out"
//./pnp.out

//in second terminal
//cp pnp_so_file.so ./plugins/


