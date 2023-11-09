#include <iostream>
#include "singleton.hpp"

using namespace hrd29;





/*************************APP IMPL*********************************/
void Foo() 
{
    YourClass* instance1 = Singleton<YourClass>::GetInstance();
  
    std::cout<<"instance1 from app is :" << instance1 <<std::endl;
   

}