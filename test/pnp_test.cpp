#include <string>   //string
#include <fstream>  //ofstream
#include <mutex>  //mutex
#include <memory>
#include <atomic>
#include <iostream>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/inotify.h>
#include <sys/eventfd.h> //eventfd
#include <sys/epoll.h>  //epoll

#include "pnp.hpp"

using namespace hrd29;


int main()
{
	DirMonitor monitor("./plugins");
	DllLoader loader(monitor.GetDispatcher());
	std::this_thread::sleep_for(std::chrono::seconds(20));

	return 0;
}
