

#ifndef _HRD29_PLUG_AND_PLAY_HPP_
#define _HRD29_PLUG_AND_PLAY_HPP_

#include <string>
#include <memory>
#include <sys/inotify.h> // inotify_event
#include <thread>

#include "eventchannel.hpp"
#include "logger.hpp"


namespace hrd29
{

class DirMonitor
{
public:
    explicit DirMonitor(std::string dir_path_ = "./plugin");
    DirMonitor(const DirMonitor& other_) = delete;
    DirMonitor& operator=(const DirMonitor& other_) = delete;
    ~DirMonitor() noexcept;

    void Add(std::string path_to_so_); // add and update
    Dispatcher<std::string>* GetDispatcher();

private:
    std::string m_dir_path;
    Dispatcher<std::string> m_dispatcher;
    std::thread m_thread;
    void ListenForEventTheard();
    Logger* logger1;


    int fd;
    int event_fd; //event_fd  for epoll
    int wd;
    bool listen;
};

class DllLoader
{
public:
    explicit DllLoader(Dispatcher<std::string>* dis_);
    DllLoader(const DllLoader& other_) = delete;
    DllLoader& operator=(const DllLoader& other_) = delete;
    ~DllLoader() noexcept;

private:
    void LoadPlugin(const std::string& path_to_so_); // ActionMethod
    std::vector<std::pair<std::string, void*>> m_loaded_directories;
    Callback<std::string, DllLoader> m_callback;
    Logger* logger2;
};

} // namespace hrd29

#endif /*_HRD29_PLUG_AND_PLAY_HPP_*/

