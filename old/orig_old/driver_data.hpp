/*******************************************************************************
 * Author: hrd29
 * Description: Header file for BeezNAS
 * Date: 03.08.23
 * Reviewer: 
 * Versions: 1.0 
*******************************************************************************/
#ifndef _HRD29_DriverData_
#define _HRD29_DriverData_

#include <iostream> 
#include <memory> //shared_ptr
#include <vector>

namespace hrd29
{
struct DriverData
{
    explicit DriverData(size_t len_); //for the buffer
    ~DriverData() = default;
    //do not generate cctor nd copy assignment
    DriverData(const DriverData& other_) = default;                       
    DriverData& operator=(const DriverData& other_) = default;

    enum Action
    {
        READ = 0,
        WRITE,
        DISC,
        FLUSH,
        TRIM 
        
    }; 

    size_t m_handle;
    Action m_action;
    size_t m_offset;
    size_t  m_len;
    std::vector<char> m_buffer;

};

}
#endif /*_HRD29_DriverData_*/

