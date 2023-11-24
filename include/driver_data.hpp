
#ifndef _HRD29_BeezNAS_Data
#define _HRD29_BeezNAS_Data

#include <iostream> 
#include <memory> //shared_ptr
#include <vector> 

namespace hrd29
{
/* representing a request or response from driver */
struct DriverData
{
    explicit DriverData(size_t len_); //for the buffer
    ~DriverData();

    enum Action
    {
        READ = 0,
        WRITE,
        DISCONN,
        FLUSH,
        STDIN
    };

    size_t m_handle;  /* uid for replay */
    Action m_action;
    size_t m_offset; /* where put the buffer in the storage */
    size_t m_len;    /* size of buffer */
    std::vector<char> m_buffer;

};
}//namespace hrd29

#endif /*_HRD29_BeezNAS_Data*/
