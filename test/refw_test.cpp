

#include "refw.hpp"
#include <iostream>
#include <cstring>      // strcspn
#include <exception> //throw
#include <csignal>          // signals 
#include <sys/epoll.h>
#include <exception>



using namespace hrd29;

#define FW_key DriverData::Action;



//IDriverCommunicator* ptr_nbd;
const size_t g_diskSize = 4096 * 4096;
const size_t DISK_SIZE = g_diskSize;
const size_t BUFFER_SIZE = 4096 * 25; 
static Logger* g_logger = nullptr;


/****************************************/
static void InitLogger();
static void InitDevice(std::shared_ptr<IDriverCommunicator>& driver_comm_ptr, char** argv);
/
static void InitStorage(std::shared_ptr<IStorage>& ram_storage_ptr);
static void InitSignals();
void WrapperDisconnecSigHandler(int sig_num);//SignalHandler
/****************************************/

int main(int argc, char* argv[])
{

    InitLogger();
    g_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DriverCommunicator - ENTER", true );

    if(2!= argc)
    {
        g_logger->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "argc Error - Missing NBD Device Path", true ); 
        return 1;
    
    }

    std::shared_ptr<IDriverCommunicator>device_ptr = nullptr;
	InitDevice(device_ptr, argv);
	std::shared_ptr<IStorage> ramStorage = nullptr;
	InitStorage(ramStorage);
    
    std::shared_ptr<DriverData> data= nullptr;
    auto mutex_wrap_ptr = std::make_shared<MutexWrap>();
	
    ReadWriteTask args(device_ptr, ramStorage, data, mutex_wrap_ptr); //cretae FW_args instance
    RequestEngineFW<DriverData::Action, ReadWriteTask> refw;
    
    refw.ConfigTask(DriverData::READ, ReadTask::Create);
    refw.ConfigTask(DriverData::WRITE, WriteTask::Create);
    refw.ConfigTask(DriverData::DISCONN, DisconnectTask::Create);
    refw.ConfigTask(DriverData::FLUSH, FlushTask::Create);
    refw.ConfigTask(DriverData::STDIN, StdinTask::Create);

    std::shared_ptr<IRequestSource<DriverData::Action, ReadWriteTask>> device_source =
    std::make_shared<NBDRequest<DriverData::Action, ReadWriteTask>>(args);

    std::shared_ptr<IRequestSource<DriverData::Action, ReadWriteTask>> stdin_source =
    std::make_shared<RequesKeyborad<DriverData::Action, ReadWriteTask>>(args);
    
    refw.ConfigRequestSource(device_source);
    refw.ConfigRequestSource(stdin_source);

    InitSignals();
    refw.Run();
       

    g_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DriverCommunicator - EXIT", true );

    return 0;
}

/*************static func**************/

static void InitLogger()
{
    try
    {
        g_logger = Singleton<Logger>::GetInstance();
    }
    catch(const std::runtime_error& ex)
    {
        std::cerr << "Logger Error: " << ex.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    g_logger->SetLogLevel(Logger::Debug);
}

static void InitDevice(std::shared_ptr<IDriverCommunicator>& driver_comm_ptr, char** argv)

{
    const std::string DEVICE_PATH = argv[1];
    std::cout <<" device path is "<< DEVICE_PATH<<std::endl;
    
    try
    {
        driver_comm_ptr = std::make_shared<NBDDriverCommunicator>(DEVICE_PATH, g_diskSize);  
    }
    catch(const std::exception& e)
    {
        
        g_logger->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, e.what(), true );
        exit(EXIT_FAILURE);
    }
    
    
}

static void InitStorage(std::shared_ptr<IStorage>& ram_storage_ptr)
{
    try
    {
        ram_storage_ptr = std::make_shared<RAMStorage>(DISK_SIZE);
    }
    catch(const std::exception& e)
    {
        
        g_logger->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, e.what(), true );
        exit(EXIT_FAILURE);
    }
}


static void InitSignals()
{
    g_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "InitSignals- enter", true );

    std::signal(SIGINT, WrapperDisconnecSigHandler);
    std::signal(SIGTERM, WrapperDisconnecSigHandler);

    g_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "InitSignals- exit", true );
}

void WrapperDisconnecSigHandler(int sig_num)
{
    g_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DisconectSignalHandler- enter", true );

    (void) sig_num;
    disc_flag = true;

    g_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DisconectSignalHandler- exit", true );
}































/*NOTE FOR ME:  when using std::signal- you do not need
sigaction and sigemptyset
cpp allows ypu to set up signalhandler for specific mask 

*/




