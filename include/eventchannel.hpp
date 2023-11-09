
/*******************************************************************************
 * Author:          hrd29
 * Description:     Header file for Publisher-Subscriber template
 * Date:            21.08.23
 * Reviewer:        ---
 * Versions:        1.0 
*******************************************************************************/

#ifndef _HRD29_EVENT_CHANNEL_HPP_
#define _HRD29_EVENT_CHANNEL_HPP_

#include <vector>
#include <iostream>
#include <algorithm>



namespace hrd29
{

template <typename MSG>
class ICallback;


template <typename MSG>
class Dispatcher
{
public:
    explicit Dispatcher() = default;
    Dispatcher(const Dispatcher& other_) = delete;
    Dispatcher& operator=(const Dispatcher& other_) = delete;
    ~Dispatcher() noexcept;

    void NotifyAll(const MSG& msg_);

private:
    friend class ICallback<MSG>;
    void Register(ICallback<MSG>* callback_);
    void UnRegister(ICallback<MSG>* callback_);

    std::vector<ICallback<MSG>*> m_callbacks;
};

template <typename MSG>
class ICallback
{
public:
    explicit ICallback(Dispatcher<MSG>* disp_);
    ICallback(const ICallback&) = delete;
    ICallback& operator=(const ICallback&) = delete;
    virtual ~ICallback() noexcept = 0;

private:
    friend class Dispatcher<MSG>;
    virtual void Notify(const MSG& msg_) = 0;
    virtual void OutOfService() = 0;
    //void Register();// do not implement- remove
    //void UnRegister();// do not implement- remove

    Dispatcher<MSG>* m_dispatcher;
};


template <typename MSG, typename OBS>
class Callback : public ICallback<MSG>
{
public:
    using ActionMethod = void(OBS::*) (const MSG& msg_);
    using StopMethod = void(OBS::*) ();

    explicit Callback(Dispatcher<MSG>* disp_, OBS& obj_,
                      ActionMethod act_, StopMethod stp_ = nullptr);
    Callback(const Callback& other_) = delete;
    Callback& operator=(const Callback& other_) = delete;
    ~Callback() override;

private:
    void Notify(const MSG& msg_) override;
    void OutOfService() override;//not mandatory

    OBS& m_obj;
    ActionMethod m_act;
    StopMethod m_stop;
};

/*************************impl of Dispatcher**********************/
template <typename MSG>
Dispatcher<MSG>:: ~Dispatcher() noexcept
{
    //ICallback<MSG>::OutOfService();


    try
    {
        for (auto it = m_callbacks.begin() ; it != m_callbacks.end(); ++it) 
        {
            (*it)->OutOfService();
            (*it)->m_dispatcher = nullptr;
        }

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    

}

template <typename MSG>
void Dispatcher<MSG>:: NotifyAll(const MSG& msg_) 
{
    for (auto it = m_callbacks.begin(); it != m_callbacks.end(); ++it) 
    {
        (*it)->Notify(msg_);
    }
}

template <typename MSG>
void Dispatcher<MSG>::Register(ICallback<MSG>* callback_)
{
    //TODO:if callback_ == nullptr
    //throw std::runtime error
    m_callbacks.push_back(callback_);
}

template <typename MSG>
void Dispatcher<MSG>::UnRegister(ICallback<MSG>* callback_)
{
    //TODO:if callback_ == nullptr
    //throw std::runtime error

    // i loooove auto- cpp finds out the type, i do not mention it
    auto it =  std::find(m_callbacks.begin(), m_callbacks.end(), callback_);
    
    if(it!= m_callbacks.end())
    {
        //erase the specific callback
        m_callbacks.erase(it);
    }

}

/*************************impl of ICallback**********************/
template <typename MSG>
ICallback<MSG>::ICallback(Dispatcher<MSG>* disp_):m_dispatcher(disp_)
{
    //Dispatcher<MSG>::Register(m_dispatcher);
    m_dispatcher->Register(this);

}

template <typename MSG>
ICallback<MSG>:: ~ICallback() noexcept // TODO: add try catch in distractors
{
    //Dispatcher<MSG>::Unregister(m_dispatcher);
    try
    {
        if(nullptr !=m_dispatcher)
        {
            m_dispatcher->UnRegister(this);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}


/*************************impl of Callback**********************/
template <typename MSG, typename OBS>
Callback<MSG, OBS>::Callback(Dispatcher<MSG>* disp_, OBS& obj_,ActionMethod act_, StopMethod stp_ ):
ICallback<MSG>(disp_), m_obj(obj_),m_act(act_), m_stop(stp_)
{
    //disp_ - is THIS! you do this by implement with ICall(disp)
    //empty

}



template <typename MSG, typename OBS>
Callback<MSG, OBS>::~Callback()
{
    //empty 
}

template <typename MSG, typename OBS>
void Callback<MSG, OBS>::Notify(const MSG& msg_)
{
    (m_obj.*m_act)(msg_);
}

template <typename MSG, typename OBS>
void Callback<MSG, OBS>::OutOfService()
{
    std::cout<<"in OutOfService"<<std::endl;
    if(nullptr!=m_stop)
    {
        //m_stop();
        (m_obj.*m_stop)();
    }

}

} // namespace hrd29

#endif /*_HRD29_EVENT_CHANNEL_HPP_*/

