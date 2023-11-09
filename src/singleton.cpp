
#include <iostream>
#include <thread>
#include "singleton.hpp"

using namespace hrd29;

void CheackMultiThreadSingelton();
void ThreadFunc();


int main()
{
    
    
    /*local test*/
    YourClass* instance1 = Singleton<YourClass>::GetInstance();
    YourClass* instance2 = Singleton<YourClass>::GetInstance();
    std::cout<<"instance1 address is :" << instance1 <<std::endl;
    std::cout<<"instance2 address is :" << instance2 <<std::endl;
   
    
   //instance1 address is :0x55fca2bcd2c0
    //instance2 address is :0x55fca2bcd2c0

    /*check multithread*/
    CheackMultiThreadSingelton();
    
    
    return 0;
}


void CheackMultiThreadSingelton()
{

    std::thread thread1(ThreadFunc);
    std::thread thread2(ThreadFunc);
  
    thread1.join();
    thread2.join();
}


void ThreadFunc()
{
    YourClass* instance_thread = Singleton<YourClass>::GetInstance();
    std::cout << "Thread number  "<< std::this_thread::get_id() << "\ninstance address: " << instance_thread << std::endl;

    /*
    
        
    Thread number  140047330309696
    instance address: 0x55fca2bcd2c0

    Thread number  140047321916992
    instance address: 0x55fca2bcd2c0
    
    */



}



