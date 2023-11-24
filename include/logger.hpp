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
    
    ~Logger();

private:
    Logger(const std::string& file_path = "./beeznas.log",Loglevel level=Error);//ctor
    friend Logger* Singleton<Logger>::GetInstance();// implementation in singelton class

    std::string m_file_path;
    std::ofstream m_ostrm; 
    Loglevel m_log_level;
    std::mutex m_mutex;
};
}

#endif /*_HRD29_LOGGER_UTILS_*/
