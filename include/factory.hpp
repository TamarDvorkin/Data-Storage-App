
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
    using Create_Func = std::function<std::shared_ptr<Base>(Args)>;
    Factory(const Factory&) = delete;
    Factory& operator=(const Factory&) = delete;
    ~Factory()= default;

    void Add(const Key& key_,  Create_Func func_);//The function also updates the key if it exits.
    std::shared_ptr<Base> Create(const Key& key_, Args args_);

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
//2 call create with specific key- looking for the key in m_umap
//3 if key found- updating associated function
//4 create function is construct instance of object

template<typename Base, typename Key, typename Args>
std::shared_ptr<Base> Factory<Base, Key, Args>::Create(const Key& key_, Args args_)
{

    return m_umap.at(key_)(args_);
}


}//namespace hrd29

#endif /*_HRD29_FACTORY_UTILS_HPP_*/
