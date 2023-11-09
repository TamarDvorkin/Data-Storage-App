/*******************************************************************************
 * Author: hrd29
 * Description: Header file for BeezNAS
 * Date: 03.08.23
 * Reviewer: 
 * Versions: 1.0 
*******************************************************************************/
#ifndef _HRD29_DriveCommun_
#define _HRD29_DriveCommun_

#include <iostream> 
#include <memory> //shared_ptr
#include <vector> 
#include <thread>
#include <signal.h>
#include "driver_data.hpp"

//devide to 3 headers:  drive_communicator.hpp, driver_data.hpp, storage.hpp

namespace hrd29
{

void ThreadFunc(size_t size,int nbd, int socket);//decouple- outside class
int read_all(int fd, char* buf, size_t count);
int write_all(int fd, char* buf, size_t count);
//int SetSigaction(int sig, const struct sigaction * act);
//void DisconnectNbd(int signal);
//void SignalHandlerFather(int socket_fd_1);


class IDriverCommunicator
{
public:
    explicit IDriverCommunicator() = default;
    virtual ~IDriverCommunicator() noexcept = default;
    IDriverCommunicator(const IDriverCommunicator& other_) = delete;                       
    IDriverCommunicator& operator=(const IDriverCommunicator& other_) = delete; 

    virtual std::shared_ptr<DriverData> ReceiveRequest() = 0; 
    virtual void SendReply(std::shared_ptr<DriverData> data_) = 0;
    virtual void Disconnect() = 0;
    virtual int GetFD() const = 0;//do as virtual
    virtual int GetSocketChild()=0;//0
    virtual int GetSocketFather()=0;//1
   
  
private:
    int m_fd;

};
//base error class -- all device
//derived error nbd-- nbd device

class NBDDriverCommunicator : public IDriverCommunicator
{
public:
//note:std::thread instead of fork
    explicit NBDDriverCommunicator(const std::string& device_path, 
                                    size_t nbd_size); //nbd_size need to multipler with size_block
    explicit NBDDriverCommunicator(const std::string& device_path, 
                                    size_t block_size,size_t num_blocks);
    ~NBDDriverCommunicator() noexcept override; 
    NBDDriverCommunicator(const NBDDriverCommunicator& other_) = delete;                       
    NBDDriverCommunicator& operator=(const NBDDriverCommunicator& other_) = delete; 

    std::shared_ptr<DriverData> ReceiveRequest() override; 
    void SendReply(std::shared_ptr<DriverData> data_) override;
    void Disconnect() override ;
    int GetFD() const override;
    int GetSocketChild()override;//0
    int GetSocketFather()override;//1
    

private:
    int socket_fd_0; 
    int socket_fd_1;
    int nbd;//this is GetFD
    std::string m_device_path;
    size_t m_nbd_size;
    std::thread child_Thread;
    std::thread parent_Thread;
    //void SignalHandlerFather(int socket_fd_1);
    //void DisconnectNbd(int signal);
    //int SetSigaction(int sig, const struct sigaction * act);



    //note: add threads parameters
    //thread *_childThread;
};

class Exception : public std::exception
{
public:
    explicit Exception(const std::string& errorMessage) : message(errorMessage) {}
    const char* what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};


#endif /*_HRD29_DriveCommun_*/
};
