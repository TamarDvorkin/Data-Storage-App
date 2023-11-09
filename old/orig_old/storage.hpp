/*******************************************************************************
 * Author: hrd29
 * Description: Header file for BeezNAS
 * Date: 03.08.23
 * Reviewer: 
 * Versions: 1.0 
*******************************************************************************/
#ifndef _HRD29_STORAGE_
#define _HRD29_STORAGE_

#include <iostream> 
#include <memory> //shared_ptr
#include <vector> 
#include "driver_data.hpp"

namespace hrd29
{
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
    explicit RAMStorage(size_t size_);
    ~RAMStorage() noexcept override;
    RAMStorage(const RAMStorage& other_) = delete;                       
    RAMStorage& operator=(const RAMStorage& other_) = delete; 
    //copy from buse
    void Read(std::shared_ptr<DriverData> data) override; //exception and type?
    void Write(std::shared_ptr<DriverData> data) override; 

private:
    std::vector<char> m_disk;
};
//namespace hrd29


#endif /*_HRD29_STORAGE_*/
}


