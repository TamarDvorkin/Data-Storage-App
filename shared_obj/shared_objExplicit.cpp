#include <iostream>
#include "singleton.hpp"

namespace hrd29
{


/*************************APP IMPL*********************************/


extern "C" 
{
    void Foo()
    { 
    YourClass* instance = hrd29::Singleton<YourClass>::GetInstance();
    std::cout<<" Foo instance address is :" << instance <<std::endl;
    }
}
}