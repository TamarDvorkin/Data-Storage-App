
#include <iostream>
//#include <thread>
#include <dlfcn.h>
#include "singleton.hpp"

using namespace hrd29;
//extern void Foo();//function declared in shared library//TODO: extern?? 
typedef void(*CallGetInsdtanceFooFunction)();



/*************************APP IMPL*********************************/


int main()
{

    YourClass* instance1 = Singleton<YourClass>::GetInstance();
  
    std::cout<<"instance1 from app(not shared) is :" << instance1 <<std::endl;



    //const char* dlsym_error;
    void* open = dlopen("./libsharedExp.so", RTLD_LAZY);
    if(0 == open)
    {
        std::cerr<<"error handling shared lib  " <<std::endl;
        return 1;
    }

    

    CallGetInsdtanceFooFunction Foo =(CallGetInsdtanceFooFunction)dlsym(open,"Foo");//Foo symbol


    Foo();

    dlclose(open);
    
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


/*

this is how to run explicit:
gb11+ -fpic -shared -rdynamic  -o libsharedExp.so shared_objExplicit.cpp //DO NOT NEED -C
gb11+  singletonExplicit.cpp -rdynamic //VERY IMPORTANT!
./a.out


result:
in getinstance
instance1 from app(not shared) is :0x5599ee7a62c0
in getinstance
Foo instance address is :0x5599ee7a62c0


*/
































