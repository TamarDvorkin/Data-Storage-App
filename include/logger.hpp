#include <string>   //string
#include <fstream>  //ofstream

#include "singleton.hpp"
#include "drive_communicator.hpp"


#ifndef _HRD29_LOGGER_UTILS_
#define _HRD29_LOGGER_UTILS_

namespace hrd29
{
class Logger
{
public:
    enum Loglevel
    {
        Error = 0,
        Info,
        Debug
    };

    void SetFile(const std::string& file_path);// set m_ostrm
    void SetLogLevel(Loglevel level);

    //for writing you can use define or funcion the idea is the same:
    //the __FILE__,__LINE__,__func__ are macros that the compiler knows to find
    //and other you need to initialize
    //log_msg- change your try catch to take the catch to LOG_WRITE with e.what()

    #define LOG_WRITE(log_level, log_msg, print_2_screen) \
    {   \
        hrd29::Log::Write(log_level,__FILE__,__LINE__,__func__,log_msg, (bool)print_2_screen)\
    }

    void Write(Loglevel level, 
            const std::string& file, 
            int line, 
            const std::string& func,
            const std::string& msg,
            bool print2screen = false
            );
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    // Dtor saying that thr file was closed via 
    ~Logger();

private:
    Logger(const std::string& file_path = "./beeznas.log",Loglevel level=Error);//ctor
    friend Logger* Singleton<Logger>::GetInstance();// implementation in singelton class

    std::string m_file_path;//path
    std::ofstream m_ostrm; //file*//ostream is like RAII know to close the file
    Loglevel m_log_level;
    std::mutex m_mutex;
};
}

#endif /*_HRD29_LOGGER_UTILS_*/