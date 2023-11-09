/*******************************************************************************
 * Author:          HRD29
 * Description:     Header file for Singleton template
 * Date:            10.08.23
 * Reviewer:        XXXX
 * Versions:        1.0 
*******************************************************************************/
#ifndef _HRD29_SINGLETON_
#define _HRD29_SINGLETON_

#include <memory>//unique_ptr
#include <mutex>
#include <atomic>

namespace hrd29
{

// usage: Your_Class instance = Singleton<Your_Class>::GetInstance(); 
// in order to use the class you need to:
//  -   set your default Ctor to private,
//  -   add  Singleton<Your_Class>::GetInstance() as friend, 
//  -   make sure the CCtor and op= are delete.



/******************implement my RAII destructor mechanisem*********/
template <typename T>
struct DtorGard //desfault- public access
{
    T* m_ptr;
    ~DtorGard()
    {
        if(m_ptr != 0)
        {
            delete m_ptr;
        }
    }
};


template <typename T>
class Singleton
{
public:
    static T* GetInstance(); 
    ~Singleton();
    Singleton(const Singleton&) = delete;
    //Singleton() = delete;
    Singleton& operator=(const Singleton&) = delete;

private:
    Singleton() = default;
   
};


template<typename T>
T* Singleton<T>::GetInstance()
{

    static std::atomic <T*>m_instance; // atomic type templete
    static std::mutex m_mutex;
    static DtorGard<T> m_dtor_gard;
    puts("in getinstance");
    T* tmp = m_instance.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);//memory barrier
    if (tmp == nullptr) 
    {
        
        std::lock_guard<std::mutex> lock(std::mutex);
        tmp = m_instance.load(std::memory_order_relaxed);//memory barrier
        if (tmp == nullptr) 
        {
            tmp = new T;// instead of singelton
            m_dtor_gard.m_ptr = tmp;
            std::atomic_thread_fence(std::memory_order_release);
            m_instance.store(tmp, std::memory_order_relaxed);
            //static SingletonDestroyer<T> singletonResource(tmp);
        
        }
    }
    return tmp;
}


// GetInstance() Function:
// Loads the value of m_instance using relaxed memory ordering. This doesn't impose any synchronization constraints on other operations.
// A memory fence (std::atomic_thread_fence(std::memory_order_acquire)) is used to ensure that previous memory operations are visible to this thread before proceeding.
// If temp is nullptr, indicating no instance exists, it acquires a lock using std::lock_guard to ensure exclusive access.
// Inside the lock, it again loads m_instance using relaxed memory ordering. If it's still nullptr, a new instance is created and assigned to temp.
// A memory fence (std::atomic_thread_fence(std::memory_order_release)) is used to ensure that the initialization of the new instance is visible to other threads.
// Finally, the new instance (or the previously created instance) is stored in m_instance, using relaxed memory ordering.
// The function returns the instance.


class YourClass
{

public:
    
    ~YourClass(){};//or generated

private:
    YourClass(){};
    YourClass(const YourClass&) = delete;
    YourClass& operator=(const YourClass&) = delete;
    template<typename T>
    friend T* Singleton<T>::GetInstance();

};


  
} // namespace hrd29

#endif /*_HRD29_SINGLETON_*/



/*HOW MAYERS WOULD HAVE DONE THAT:
from here- ods and sods 

https://sourcemaking.com/design_patterns/to_kill_a_singleton

Long ago, in a galaxy far, far away, Scott Meyers posited the following: My version of Singleton is quite similar to yours, but instead of using a class static and having Instance return a pointer, I use a function static and return a reference:

Singleton &Singleton::Instance() // STATIC FUNC
{
 static Singleton s;
 return s;
}

but... all the problems of ,multithread  etc'
MULTITHREAD-  UNDEFINED BEHAVIER FOR STATIC+ULTITHREAD IN CPP98







*/