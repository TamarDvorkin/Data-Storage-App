/*******************************************************************************
 * Author: hrd29
 * Description: Header file for BeezNAS Storage
 * Date: 03.08.23
 * Reviewer: 
 * Versions: 1.0 
*******************************************************************************/
#ifndef _HRD29_BeezNAS_Storage_
#define _HRD29_BeezNAS_Storage_

#include <iostream> 
#include <memory> //shared_ptr
#include <vector> 
#include "driver_data.hpp"

namespace hrd29
{
/* Interface class for storage.
    Defines action (read + write) */
class IStorage
{
public:
    explicit IStorage()= default;
    virtual ~IStorage() = default;
    IStorage(const IStorage& other_) = delete; /* =delete : block the compiler from generate the function */                               
    IStorage& operator=(const IStorage& other_) = delete; 

    virtual void Read(std::shared_ptr<DriverData> data) = 0; //exception and type?
    virtual void Write(std::shared_ptr<DriverData> data) = 0; 
};


/* Implementation of IStorage.
    Manages storage using RAM storage. */
class RAMStorage : public IStorage
{
public:
    explicit RAMStorage(size_t size_); /* size of m_disk */
    ~RAMStorage() noexcept override;
    RAMStorage(const RAMStorage& other_) = delete;                       
    RAMStorage& operator=(const RAMStorage& other_) = delete; 

    void Read(std::shared_ptr<DriverData> data) override; //exception and type?
    void Write(std::shared_ptr<DriverData> data) override; 

private:
    std::vector<char> m_disk; /* = storage */
};

}//namespace hrd29

#endif /*_HRD29_BeezNAS_Storage_*/