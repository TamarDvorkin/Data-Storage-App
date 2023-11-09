/*******************************************************************************
 * Author: HRD29
 * Description: Header file for Factory
 * Date: 019.07.2023
 * Reviewer: ---
 * Versions: 1.0 
*******************************************************************************/
#include <functional>
#include <unordered_map>
#include "singleton.hpp"
#include "memory"

#ifndef _HRD29_FACTORY_UTILS_HPP_
#define _HRD29_FACTORY_UTILS_HPP_

namespace hrd29
{

template<typename Base, typename Key, typename Args>
class Factory
{
public:
    using Create_Func = std::function<std::shared_ptr<Base>(Args)>;//New feature - instaed of typedef
    Factory(const Factory&) = delete;
    Factory& operator=(const Factory&) = delete;
    ~Factory()= default;

    void Add(const Key& key_,  Create_Func func_);//The function also updates the key if it exits.
    std::shared_ptr<Base> Create(const Key& key_, Args args_);//Can throw an exception

private:
    Factory() = default;
    friend Factory* Singleton<Factory<Base, Key, Args>>::GetInstance();
    std::unordered_map<Key, Create_Func> m_umap;//associate key with correspond function
    
};

template<typename Base, typename Key, typename Args>
void Factory<Base,Key,Args>::Add(const Key& key_,  Create_Func func_)
{
    m_umap[key_] = func_;
}


//how m_umap member is used:
//1 add key and create function using "add"
//2 call create wuth specific key- looking for the key in m_umap
//3 uf key found- upfating associated function
//4 create function is construct instance of object

template<typename Base, typename Key, typename Args>
std::shared_ptr<Base> Factory<Base, Key, Args>::Create(const Key& key_, Args args_)
{
    //auto- the compiler will deduce automaticlly the type

     //short way: return m_umap.at(key_)(args_);

    //long impl with iterators:
    /*auto it = m_umap.find(key_);
    if(it!= m_umap.end())
    {
        Create_Func CreateFunction = it->second; //second is key(first):funct(second)
        return CreateFunction(args_);
    }
    else
    {
        throw std::runtime_error("key not founded");
    }*/

    return m_umap.at(key_)(args_);

    //Returns a reference to the mapped value of the element with key equivalent to key. 
    //If no such element exists, an exception of type std::out_of_range is thrown.


}


}//namespace hrd29

#endif /*_HRD29_FACTORY_UTILS_HPP_*/
