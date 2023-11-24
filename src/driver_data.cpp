#include <stdio.h>
#include "driver_data.hpp"

namespace hrd29
{
    
    
    DriverData::DriverData(size_t len_) : m_len(len_), m_buffer(len_)
    {}

    DriverData::~DriverData() 
    {}
}



