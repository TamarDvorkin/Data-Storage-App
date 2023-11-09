#include <string>   //string
#include <fstream>  //ofstream
#include <mutex>  //mutex
#include <memory>
#include <atomic>
#include <iostream>
#include <thread>

#include "singleton.hpp"
#include "logger.hpp"

namespace hrd29
{

    Logger::~Logger()
    {
        std::cout<< "finished logging your program "<<std::endl;
    }

    Logger::Logger(const std::string& file_path,Loglevel level)
    {
        m_log_level = level;
        m_file_path = file_path; 
        //open ostream
        //throw if fail
        m_ostrm.open(file_path, std::ios::app);
        if(!m_ostrm.is_open())
        {
            throw MyException("Failed open file in constructor function");
        }
    }

    //in public first: 
    void Logger::SetFile(const std::string& file_path)
    {
        static std::mutex m_mutex;
        std::lock_guard<std::mutex> lock(std::mutex);
        //close m_ostrem
        m_ostrm.close();//calls std::close
        m_file_path = file_path; 
        //for now i do not use: std::ofstream ostrm(file_path, std::ios::binary);
        //open m_ostrem
        m_ostrm.open(m_file_path, std::ios::app);// app- append to the file
        //This flag ensures that the new log entries are appended to the existing file rather than overwriting it.
        if(!m_ostrm.is_open())
        {
            throw MyException("Failed open file in SetFile function");
        }
    }

    void Logger::SetLogLevel(Loglevel level)
    {
        //you can do safe thread like this:
         //m_log_level.store(level, std::memory_order_seq_cst);
        m_log_level = level;
    }

    void Logger::Write(Loglevel level, const std::string& file, int line, const std::string& func,const std::string& msg,bool print2screen) 
    {
        
        // file,line,func-is already set to macro by define
        //lock
        //check the level- and if equall or bigger msg- write to file
        //check if(print2screen)- std::out

        //adding- DateTime and ThreadId
        time_t now = time(0);
        struct tm * time_info = localtime(&now);
        const size_t buff_size = 25;
        char buffer[buff_size];//buffer for DateTime 
        strftime(buffer, buff_size, "%x %X", time_info);//"%Y-%m-%d %H:%m:%S"
        std::thread::id thread_id = std::this_thread::get_id();
        

        if(level <= m_log_level) 
        {//takes from the macro
        //lock- Mutex
            std::lock_guard<std::mutex> lock(std::mutex);
            m_ostrm << thread_id << " " << buffer <<level<<":"<<file<<":"<<func<<":"<<line<<":"<<"["<<msg<<"]"<<"\n";
            if(print2screen)
            {
                std::cout<< thread_id << " " << buffer<<level<<":"<<file<<":"<<func<<":"<<line<<":"<<"["<<msg<<"]"<<"\n";
            }
        }
    }


}
