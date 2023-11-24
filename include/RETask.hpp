

#ifndef _HRD29_Request_Task_
#define _HRD29_Request_Task_

#include <utility>//std::pair
#include <unistd.h> // STDIN_FILENO
#include <cstring>  // strcmp
#include <cerrno>

#include "interface_thread_task.hpp"
#include "driver_data.hpp"
#include "drive_communicator.hpp"
#include "storage.hpp"
#include "logger.hpp"


namespace hrd29
{

static bool StdinQuit(Logger* m_logger);

static void SendDriverData(const std::shared_ptr<IDriverCommunicator>& driver_comm_ptr, std::shared_ptr<DriverData>& driver_data_ptr,
Logger* logger);
//

/****************ReadWriteTask args Struct****************/

struct MutexWrap
{
    std::mutex m_reply_mutex;
};


struct ReadWriteTask
{
  
    ReadWriteTask() = default;
    
    ReadWriteTask(const std::shared_ptr<IDriverCommunicator> driver_communicator,
            const std::shared_ptr<IStorage> ram_storage,
            std::shared_ptr<DriverData> driver_data,
            std::shared_ptr<MutexWrap> mutex_wrap);
    
    std::shared_ptr<IDriverCommunicator> m_driver_communicator;
    std::shared_ptr<IStorage> m_ram_storage;
    std::shared_ptr<DriverData> m_driver_data;
    std::shared_ptr<MutexWrap> m_mutex_wrap;

};



ReadWriteTask::ReadWriteTask(const std::shared_ptr<IDriverCommunicator> driver_communicator,
                const std::shared_ptr<IStorage> ram_storage,
                std::shared_ptr<DriverData> driver_data,
                std::shared_ptr<MutexWrap> mutex_wrap) :
                m_driver_communicator(driver_communicator),
                m_ram_storage(ram_storage),
                m_driver_data(driver_data),
                m_mutex_wrap(mutex_wrap)
{}



class RETask: public ITask 
{
public:
    RETask (Priority priority_ = MEDIUM); //Priority: LOW, MEDIUM, HIGH
    virtual ~RETask() noexcept = 0 ;
    RETask(const RETask& other_) = delete;
    RETask& operator=(const RETask& other_) = delete;
    virtual void Execute()= 0;// user need to give me Execute function
private:
};
//ITask - father, RETask - child , ReadTask - grandchild

class ReadTask: public RETask
{
public:
    static std::shared_ptr<RETask> Create(const ReadWriteTask &args_);// create takes my struct
    ~ReadTask() override = default;
    ReadTask(const ReadTask& other_) = delete;
    ReadTask& operator=(const ReadTask& other_) = delete;
    void Execute() override;
    
private:

    explicit ReadTask(const ReadWriteTask& args_, Priority priority_ = MEDIUM);   
    ReadWriteTask args_m;
    Logger* m_logger;
};


class WriteTask: public RETask
{
public:

    static std::shared_ptr<RETask> Create(const ReadWriteTask& args_);// create takes my struct

    ~WriteTask() override = default;
    WriteTask(const WriteTask& other_) = delete;
    WriteTask& operator=(const WriteTask& other_) = delete;
    void Execute() override;
    
private:

    explicit WriteTask(const ReadWriteTask& args_, Priority priority_ = MEDIUM);  
    ReadWriteTask args_m;
    Logger* m_logger;


};

class DisconnectTask : public RETask
{
public:
    static std::shared_ptr<RETask> Create(const ReadWriteTask& args_);
    static std::shared_ptr<RETask> CreateDefault();

    DisconnectTask(const DisconnectTask& other_) = delete;
    DisconnectTask& operator=(const DisconnectTask& other_) = delete;
    ~DisconnectTask() override = default;

    void Execute() override;

private:
    explicit DisconnectTask(const ReadWriteTask& args_, Priority priority_ = HIGH);
    explicit DisconnectTask(Priority priority_ = HIGH);

    ReadWriteTask args_m;
    Logger* m_logger;
};

class FlushTask : public RETask
{
public:
    static std::shared_ptr<RETask> Create(const ReadWriteTask& args_);

    FlushTask(const FlushTask& other_) = delete;
    FlushTask& operator=(const FlushTask& other_) = delete;
    ~FlushTask() override = default;

    void Execute() override;

private:
    explicit FlushTask(const ReadWriteTask& args_, Priority priority_ = MEDIUM);

    ReadWriteTask args_m;
    Logger* m_logger;
};

class StdinTask : public RETask
{
public:
    static std::shared_ptr<RETask> Create(const ReadWriteTask& args_);

    StdinTask(const StdinTask& other_) = delete;
    StdinTask& operator=(const StdinTask& other_) = delete;
    ~StdinTask() override = default;

    void Execute() override;

private:
    explicit StdinTask(const ReadWriteTask& args_, Priority priority_ = HIGH);

    ReadWriteTask args_m;
    Logger* m_logger;

};




/*************************************RETask IMP****************************/

RETask::RETask(Priority priority_) : ITask(priority_)
{}

RETask::~RETask() noexcept
{}


/*************************************ReadTask IMP****************************/



ReadTask::ReadTask(const ReadWriteTask& args_, Priority priority_):
args_m(args_), RETask(priority_),m_logger(Singleton<Logger>::GetInstance())
{}
 


std::shared_ptr<RETask> ReadTask::Create(const ReadWriteTask& args_)
{
    std::shared_ptr<RETask> task_ptr(new ReadTask(args_));

    return task_ptr;
}


void ReadTask::Execute()
{
    m_logger->Write( hrd29::Logger::Info,__FILE__, __LINE__, __func__, "executing READ request", true );
    
    args_m.m_ram_storage->Read(args_m.m_driver_data);
    std::lock_guard<std::mutex> lock_reply(args_m.m_mutex_wrap->m_reply_mutex);
    SendDriverData(args_m.m_driver_communicator, args_m.m_driver_data, m_logger);
}

/*************************************WriteTask IMP****************************/

WriteTask::WriteTask(const ReadWriteTask& args_, Priority priority_):
RETask(priority_), args_m(args_),m_logger(Singleton<Logger>::GetInstance())
{}



void WriteTask::Execute()
{
    //call to storage->Read()
    m_logger->Write( hrd29::Logger::Info,__FILE__, __LINE__, __func__, "executing WRITE request", true );
    args_m.m_ram_storage->Write(args_m.m_driver_data);

    std::lock_guard<std::mutex> lock_reply(args_m.m_mutex_wrap->m_reply_mutex);
    //args_m.m_driver_communicator->SendReply(args_m.m_driver_data);
    SendDriverData(args_m.m_driver_communicator, args_m.m_driver_data, m_logger);
}

std::shared_ptr<RETask>WriteTask::Create(const ReadWriteTask& args_)//create instance of task
{
    std::shared_ptr<RETask> task_ptr(new WriteTask(args_));
    return task_ptr;
}


/*************************************Disconnect IMP****************************/

DisconnectTask::DisconnectTask(const ReadWriteTask& args_, Priority priority_) :
RETask(priority_), args_m(args_), m_logger(Singleton<Logger>::GetInstance())
{}

DisconnectTask::DisconnectTask(Priority priority_):
RETask(priority_), m_logger(Singleton<Logger>::GetInstance())
{}


std::shared_ptr<RETask> DisconnectTask::Create(const ReadWriteTask& args_)
{
    std::shared_ptr<RETask> task_ptr(new DisconnectTask(args_));

    return task_ptr;
}

std::shared_ptr<RETask> DisconnectTask::CreateDefault()
{
    std::shared_ptr<RETask> task_ptr(new DisconnectTask());

    return task_ptr;
}

void DisconnectTask::Execute()
{

    m_logger->Write( hrd29::Logger::Info,__FILE__, __LINE__, __func__, "executing DISCONNECT request", true );
}

/*************************************Disconnect IMP****************************/

FlushTask::FlushTask(const ReadWriteTask& args_, Priority priority_) :
RETask(priority_), args_m(args_), m_logger(Singleton<Logger>::GetInstance())
{}

std::shared_ptr<RETask> FlushTask::Create(const ReadWriteTask& args_)
{
    std::shared_ptr<RETask> task_ptr(new FlushTask(args_));

    return task_ptr;
}

void FlushTask::Execute()
{
    m_logger->Write( hrd29::Logger::Info,__FILE__, __LINE__, __func__, "executing FLUSH request", true );

    std::lock_guard<std::mutex> lock_reply(args_m.m_mutex_wrap->m_reply_mutex);
    SendDriverData(args_m.m_driver_communicator, args_m.m_driver_data, m_logger);
}



/*************************************StdinTask IMP****************************/

StdinTask::StdinTask(const ReadWriteTask& args_, Priority priority_):
RETask(priority_), args_m(args_), m_logger(Singleton<Logger>::GetInstance())
{}

std::shared_ptr<RETask> StdinTask::Create(const ReadWriteTask& args_)
{
    std::shared_ptr<RETask> task_ptr(new StdinTask(args_));

    return task_ptr;
}

void StdinTask::Execute()
{
    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "executing FLUSH request", true);
    if(true == StdinQuit(m_logger))
    {  
        exit(EXIT_SUCCESS); 
    }
    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "executing FLUSH request", true);

}

/******************static func*****************/
static bool StdinQuit(Logger* m_logger)
{

    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "StdinQuit - Enter", true);

    char buffer[64] = {0};
    ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));
    if(-1 == bytes_read)
    {
        exit(1);
    }

    *(buffer + bytes_read) = '\0';

    if((0 == strcmp(buffer, "q\n")) || (0 == strcmp(buffer, "Q\n")))
    {
        m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "Recieved Stdin Quit Command", true);
        
        return true;
    }

    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "Stdin Quit Exit", true);

    return false;
}

static void SendDriverData
(const std::shared_ptr<IDriverCommunicator>& driver_comm_ptr, std::shared_ptr<DriverData>& driver_data_ptr,
Logger* logger)
{
 
    logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "SendDriverData - Enter", true);

    try
    {
        driver_comm_ptr->SendReply(driver_data_ptr);
    }
    catch(const std::exception& e)
    {
        logger->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, e.what(), true);
        exit(EXIT_FAILURE);
    }
    
    logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "SendDriverData - Exit", true);
}


}

#endif //_HRD29_Request_Task_
