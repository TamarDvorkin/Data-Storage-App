#include <stdio.h>
#include <iostream>
#include <cstring>      // strcspn
#include <exception> //throw
#include <signal.h>          /* signals */
#include <sys/epoll.h>
#include "drive_communicator.hpp" //testclass

#include "logger.hpp"
#include "factory.hpp"

using namespace hrd29;

class Base 
{
public:
    virtual void PrintType() const 
    {
        std::cout << "Base" << std::endl;
    }
    virtual ~Base()
    {
        std::cout << "dtor base" << std::endl;
    }
};

class Derived1 : public Base 
{
public:
    void PrintType() const override 
    {
        std::cout << "Derived1" << std::endl;
    }
};

class Derived2 : public Base 
{
public:
    void PrintType() const override 
    {
        std::cout << "Derived2" << std::endl;
    }
};

// Define the creation functions for the derived classes
std::shared_ptr<Base> CreateDerived1(int args) 
{
    return std::make_shared<Derived1>();//calling Ctor with new
}

std::shared_ptr<Base> CreateDerived2(int args) 
{
    return std::make_shared<Derived2>();//calling Ctor with new
}


int main()
{
    //template<typename Base, typename Key, typename Args>
    Factory<Base, std::string, int> *my_factory = Singleton<Factory<Base, std::string, int>>::GetInstance();
    
    // Register the creation functions with keys
    my_factory->Add("Derived1", CreateDerived1);
    my_factory->Add("Derived2", CreateDerived2);

    try 
    {
        // Create instances using the factory
        std::shared_ptr<Base> firstobj = my_factory->Create("Derived1", 15);
        firstobj->PrintType();

        std::shared_ptr<Base> secondobj = my_factory->Create("Derived2", 58);
        secondobj->PrintType();

        // Try to create an instance with an unknown key
        std::shared_ptr<Base> Thirsobj = my_factory->Create("TestingKey", 999);
    }
    
    catch (const std::exception& e) 
    {
        std::cerr << "Exception: " << e.what() << std::endl;//this test throw exception in purpose
    }


}
