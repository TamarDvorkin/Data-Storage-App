#include <stdio.h>
#include <iostream>
#include <cstring>      // strcspn
#include <exception> //throw
#include <signal.h>          /* signals */
#include <sys/epoll.h>
#include "drive_communicator.hpp" //testclass

#include "logger.hpp"
#include "singleton.hpp"

using namespace hrd29;

void CheackMultiThreadLogger();
void ThreadFunc();


int main()
{
    
    
    /*local test*/
    Logger* instance1 = Singleton<Logger>::GetInstance();
    Logger* instance2 = Singleton<Logger>::GetInstance();
    std::cout<<"instance1 address is :" << instance1 <<std::endl;
    std::cout<<"instance2 address is :" << instance2 <<std::endl;
   //call write function

   instance1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "main thread error level", true );

    
   //instance1 address is :0x55fca2bcd2c0
    //instance2 address is :0x55fca2bcd2c0

    /*check multithread*/
    CheackMultiThreadLogger();
    
    
    return 0;
}


void CheackMultiThreadLogger()
{

    std::thread thread1(ThreadFunc);
    std::thread thread2(ThreadFunc);
  
    thread1.join();
    thread2.join();
}


void ThreadFunc()
{
    Logger* instance_thread = Singleton<Logger>::GetInstance();
    std::cout << "Thread number  "<< std::this_thread::get_id() << "\ninstance address: " << instance_thread << std::endl;
    std::cout <<"logger print from therad" <<std::endl;
   instance_thread->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "main thread error level", true );



}
