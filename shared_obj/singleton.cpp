
#include <iostream>
//#include <thread>
#include <dlfcn.h>
#include "singleton.hpp"

using namespace hrd29;
extern void Foo();//function declared in shared library//TODO: extern?? 




/*************************APP IMPL*********************************/


int main()
{
    
    YourClass* instance1 = Singleton<YourClass>::GetInstance();
  
    std::cout<<"instance1 from app is :" << instance1 <<std::endl;
    Foo();
   
    
    return 0;
}










/*this is how to run implicit*/
/*

gb11+ ./shared_obj.cpp -c -fPIC

gb11+ -shared ./shared_obj.o -o ./libsingleton.so

gb11+ -fPIC ./singleton.cpp -lsingleton -L. -Wl,--rpath=./

./a.out



the result:
in getinstance
instance1 from app is :0x55b346a3f2c0
in getinstance
instance1 from app is :0x55b346a3f2c0


*/
































