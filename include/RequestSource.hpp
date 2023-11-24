
#ifndef _HRD29_Request_Source_
#define _HRD29_Request_Source_

#include <utility>//std::pair
#include <fcntl.h>
#include <unistd.h>
#include "logger.hpp"
#include "drive_communicator.hpp"
#include "driver_data.hpp"
#include "storage.hpp"



namespace hrd29
{

static void ReceiveDriverData
(const std::shared_ptr<IDriverCommunicator>& driver_comm_ptr, std::shared_ptr<DriverData>& driver_data_ptr,
Logger* logger);


template< typename KEY, typename ARGS>
class IRequestSource
{
public:
    explicit IRequestSource()= default;
    virtual ~IRequestSource() noexcept =0;
    IRequestSource(const IRequestSource& other_) = delete;
    IRequestSource& operator=(const IRequestSource& other_) = delete; 

    virtual std::pair<KEY,ARGS>  Read() = 0; 
    virtual int GetFd()=0;

private:

};

template< typename KEY, typename ARGS>
class RequesKeyborad : public IRequestSource<KEY,ARGS>
{
    
public:
    explicit RequesKeyborad(ARGS& args_);
    std::pair<KEY, ARGS> Read() override;
    int GetFd() override;

private:
    
    ARGS args_m;
    Logger* m_logger;


};


template< typename KEY, typename ARGS>
class NBDRequest : public IRequestSource<KEY,ARGS>//ARGS = STD::SHARED_PTR<RETask>
{

public:
    explicit NBDRequest(const ARGS& args_);
    NBDRequest(const NBDRequest& other_) = delete;
    NBDRequest& operator=(NBDRequest& other_) = delete;
    ~NBDRequest() override = default;

    std::pair<KEY, ARGS> Read() override;
    int GetFd() override;

private:

    ARGS args_m;
    Logger* m_logger;

};


/**************************************************************IRequestSource IMP***************************************************************/

template< typename KEY, typename ARGS>
IRequestSource<KEY,ARGS>::~IRequestSource() noexcept
{}


/***************************************************************RequesKeyborad IMP************************************************************/

template< typename KEY, typename ARGS>
RequesKeyborad<KEY, ARGS>::RequesKeyborad(ARGS& args_):args_m(args_), m_logger(Singleton<Logger>::GetInstance())
{
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
}

template< typename KEY, typename ARGS>
std::pair<KEY, ARGS> RequesKeyborad<KEY, ARGS>::Read()
{

    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "StdinSource::Read - Enter", true );
    return std::make_pair(DriverData::STDIN, args_m);
}

template< typename KEY, typename ARGS>
int RequesKeyborad<KEY, ARGS> ::GetFd()
{
    return STDIN_FILENO;
}


/********************************************************NBDRequest IMP*****************************************************************/

template< typename KEY, typename ARGS>
NBDRequest<KEY, ARGS>::NBDRequest(const ARGS& args_):args_m(args_), m_logger(Singleton<Logger>::GetInstance())
{
    fcntl(this->GetFd(), F_SETFL, fcntl(this->GetFd(), F_GETFL) & ~O_NONBLOCK);
}

template< typename KEY, typename ARGS>
std::pair<KEY, ARGS> NBDRequest<KEY, ARGS>::Read()
{
    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DeviceSource::Read - ENTER", true );
    ReceiveDriverData(args_m.m_driver_communicator, args_m.m_driver_data, m_logger);
    m_logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "DeviceSource::Read - Exit", true );
    return std::make_pair(args_m.m_driver_data->m_action, args_m);

}



template< typename KEY, typename ARGS>
int NBDRequest<KEY, ARGS> ::GetFd()
{
    return args_m.m_driver_communicator->GetFD();
}


/**********************************************************static func************************************************************************/
static void ReceiveDriverData
(const std::shared_ptr<IDriverCommunicator>& driver_comm_ptr, std::shared_ptr<DriverData>& driver_data_ptr,
Logger* logger)
{
    
    logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "ReceiveDriverData - Enter", true );
    try
    {
        driver_data_ptr = driver_comm_ptr->ReceiveRequest();
    }
    catch(const std::exception& e)
    {
        logger->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, e.what(), true);
        exit(EXIT_FAILURE);
    }
    

    logger->Write( hrd29::Logger::Debug,__FILE__, __LINE__, __func__, "ReceiveDriverData - Exit", true );


}



}

#endif //_HRD29_Request_Source_
