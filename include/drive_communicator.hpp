/*******************************************************************************
 * Author: hrd29
 * Description: Header file for BeezNAS
 * Date: 03.08.23
 * Reviewer: 
 * Versions: 1.0 
*******************************************************************************/
#ifndef _HRD29_BeezNAS_DriveComm
#define _HRD29_BeezNAS_DriveComm

#include <iostream>
#include <memory> //shared_ptr
#include <thread>           /* threads */
#include "driver_data.hpp" // DriverData
#include "logger.hpp"



namespace hrd29
{
/* Interface class for communication with the NBD driver.
    Defines methods for receiving requests from specific driver, sending replies back to specific driver, and handling disconnection */
class IDriverCommunicator
{
public:
    explicit IDriverCommunicator() = default;
    virtual ~IDriverCommunicator() noexcept = default;
    IDriverCommunicator(const IDriverCommunicator& other_) = delete; /* =delete : block the compiler from generate the function */                      
    IDriverCommunicator& operator=(const IDriverCommunicator& other_) = delete; 

    virtual std::shared_ptr<DriverData> ReceiveRequest() = 0; 
    virtual void SendReply(std::shared_ptr<DriverData> data_) = 0;
    virtual void Disconnect() = 0;
    virtual int GetFD() const = 0; /* file descriptor of the request - for the epoll function */
    Logger* logger1;//instance1


private:
};


/* Implementation of IDriverCommunicator.
    Manages communication with the NBD driver over the socket. */
class NBDDriverCommunicator : public IDriverCommunicator
{
public:
//note:std::thread instead of fork
    explicit NBDDriverCommunicator(const std::string& device_path , 
                                    size_t nbd_size); //nbd_size need to be multiplied of size_block
    explicit NBDDriverCommunicator(const std::string& device_path, 
                                    size_t block_size,size_t num_blocks);
    ~NBDDriverCommunicator() noexcept override; 
    NBDDriverCommunicator(const NBDDriverCommunicator& other_) = delete;                       
    NBDDriverCommunicator& operator=(const NBDDriverCommunicator& other_) = delete; 

    std::shared_ptr<DriverData> ReceiveRequest() override;
    void SendReply(std::shared_ptr<DriverData> data_) override;
    void Disconnect() override;
    int GetFD() const override;
    


private:
    int m_socket_fd;
    int m_main_fd;

    // int nbd_fd;
    // int sock_pair[2];   /* sock_pair[1] is the NBD driver socket and sock_pair[0] is the socket we can use for communication. */
    // u_int64_t size;
    std::thread background_thread;
    // void ChildThread();
    // void ParentFunction();
    //note: add threads parameters
};


class MyException : public std::exception
{
public:
    explicit MyException(const std::string& errorMessage) : message(errorMessage) {}
    const char* what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};

}//namespace hrd29

#endif /*_HRD29_BeezNAS_DriveComm*/