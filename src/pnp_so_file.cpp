#include <iostream>

void  __attribute__((constructor)) ctor(); //shared library object
int Print();

int Print()
{
    std::cout << "Hy I'm A Shared Object" << std::endl;
    return 0;
}

void ctor()
{
    std::cout << "Shared Object Ctor" << std::endl;
    Print();
}