/*******************************************************************************
 * Author: hrd29
 * Description: Header file for BeezNAS
 * Date: 03.08.23
 * Reviewer: 
 * Versions: 1.0 
*******************************************************************************/
#ifndef _HRD29_BeezNAS_
#define _HRD29_BeezNAS_

#include <iostream> 
#include <memory> //shared_ptr
#include <vector> 
//devide to 3 headers:  drive_communicator.hpp, driver_data.hpp, storage.hpp


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
    int GetFD() const;
  
private:
};

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

private:
    int socket_fd; 
    //note: add threads parameters
};

struct DriverData
{
    explicit DriverData(size_t len_); //for the buffer
    ~DriverData();

    enum Action
    {
        READ = 0,
        WRITE,
        FLUSH
    }; 
    size_t m_handle;
    Action m_action;
    size_t m_offset;
    size_t m_len;
    std::vector<char> m_buffer; 
};

class IStorage
{
public:
    explicit IStorage()= default;
    virtual ~IStorage() = default;
    IStorage(const IStorage& other_) = delete;                       
    IStorage& operator=(const IStorage& other_) = delete; 

    virtual void Read(std::shared_ptr<DriverData> data) = 0; //exception and type?
    virtual void Write(std::shared_ptr<DriverData> data) = 0; 
};

class RAMStorage : public IStorage
{
public:
    explicit RAMStorage(size_t size_);//vector size
    ~RAMStorage() noexcept override;//defualt distructor- empty
    RAMStorage(const RAMStorage& other_) = delete;                       
    RAMStorage& operator=(const RAMStorage& other_) = delete; 

    void Read(std::shared_ptr<DriverData> data) override; //exception and type?
    void Write(std::shared_ptr<DriverData> data) override; 

private:
    std::vector<char> m_disk;
};
}//namespace hrd29

#endif /*_HRD29_BeezNAS_*/