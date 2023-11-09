
#include <iostream> 
#include <fstream>
#include <string>
#include <memory> //shared_ptr
#include <vector> 
#include "drive_communicator.hpp"
#include "driver_data.hpp"
#include <cstddef>
#include <sys/socket.h> /*socketpair*/
#include <stdio.h> /*io*/
#include <fcntl.h>/*open*/
#include <bits/stdc++.h> 
#include <thread>
#include <sys/ioctl.h>/*ioctl*/
#include <linux/nbd.h>/*nbd*/

namespace hrd29
{


DriverData::DriverData(size_t len_):m_len(len_)
{};


}



