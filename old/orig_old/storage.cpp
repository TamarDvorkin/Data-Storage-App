#include <iostream> 
#include <memory> //shared_ptr
#include <vector> 
#include "driver_data.hpp"
#include "storage.hpp"
#include <cstring>

namespace hrd29
{
 RAMStorage ::RAMStorage(size_t size_):m_disk(size_)
 {}

 RAMStorage ::~RAMStorage()
 {}

void RAMStorage :: Read(std::shared_ptr<DriverData> data)
{
    size_t offset = data->m_offset;
    size_t len = data->m_len;
    std::memcpy(&(data->m_buffer[0]), &m_disk[offset], len); //do memcopy
} //exception and type?

void RAMStorage :: Write(std::shared_ptr<DriverData> data)
{
    size_t offset = data->m_offset;
    size_t len = data->m_len;
    std::memcpy(&m_disk[offset], &(data->m_buffer[0]),len); 

}

}