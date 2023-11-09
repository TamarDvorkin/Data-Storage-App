
/*******************************************************************************
 * Author:          RotemT
 * Description:     Header file for Publisher-Subscriber template
 * Date:            21.08.23
 * Reviewer:        ---
 * Versions:        1.0 
*******************************************************************************/

#ifndef _HRD29_EVENT_CHANNEL_HPP_
#define _HRD29_EVENT_CHANNEL_HPP_

#include <vector>
#include <exception>
#include <algorithm> // std::for_each, std::remove

#include "logger.hpp"


namespace hrd29
{

static bool g_debug_print = false;
static Logger* logger_event = Singleton<Logger>::GetInstance();

template <typename MSG>
class ICallback;

/**************** Dispatcher Class ****************/

template <typename MSG>
class Dispatcher
{
public:
    explicit Dispatcher() = default;
    Dispatcher(const Dispatcher& other_) = delete;
    Dispatcher& operator=(const Dispatcher& other_) = delete;
    ~Dispatcher() noexcept = default;

    void NotifyAll(const MSG& msg_);

private:
    friend class ICallback<MSG>;
    void Register(ICallback<MSG>* callback_);
    void UnRegister(ICallback<MSG>* callback_);

    std::vector<ICallback<MSG>*> m_callbacks;
};


// template <typename MSG>
// void Dispatcher<MSG>::NotifyAll(const MSG& msg_)
// {
//     size_t size = m_callbacks.size();

//     for(size_t i = 0; i < size; ++i)
//     {
//         m_callbacks.at(i)->Notify(msg_);
//     }
// }

template <typename MSG>
void Dispatcher<MSG>::NotifyAll(const MSG& msg_)
{
    LOG_WRITE(logger_event, Logger::DEBUG, "NotifyAll - Enter", g_debug_print);

    std::for_each(m_callbacks.begin(), m_callbacks.end(), [&msg_](ICallback<MSG>* callback)
                                                          { callback->Notify(msg_); });
    
    LOG_WRITE(logger_event, Logger::DEBUG, "NotifyAll - Exit", g_debug_print);
}

template <typename MSG>
void Dispatcher<MSG>::Register(ICallback<MSG>* callback_)
{
    LOG_WRITE(logger_event, Logger::DEBUG, "Register - Enter", g_debug_print);

    try
    {
        m_callbacks.emplace_back(callback_);
    }
    catch(const std::exception& ex)
    {
        LOG_WRITE(logger_event, Logger::ERROR, ex.what(), true);
        throw("Dispatcher Register: " + std::string(ex.what()));
    }

    LOG_WRITE(logger_event, Logger::DEBUG, "Register - Exit", g_debug_print);
}

template <typename MSG>
void Dispatcher<MSG>::UnRegister(ICallback<MSG>* callback_)
{
    LOG_WRITE(logger_pool, Logger::DEBUG, "UnRegister - Enter", g_debug_print);

    m_callbacks.erase(std::remove(m_callbacks.begin(), m_callbacks.end(), callback_), m_callbacks.end());

    LOG_WRITE(logger_pool, Logger::DEBUG, "UnRegister - Exit", g_debug_print);
}




/**************** ICallback Class ****************/

template <typename MSG>
class ICallback
{
public:
    explicit ICallback(Dispatcher<MSG>* dispatcher_);
    ICallback(const ICallback&) = delete;
    ICallback& operator=(const ICallback&) = delete;
    virtual ~ICallback() noexcept = 0;

private:
    friend class Dispatcher<MSG>;
    virtual void Notify(const MSG& msg_) = 0;
    virtual void OutOfService() = 0;

    Dispatcher<MSG>* m_dispatcher;
};


template <typename MSG>
ICallback<MSG>::ICallback(Dispatcher<MSG>* dispatcher_) : m_dispatcher(dispatcher_)
{
    LOG_WRITE(logger_pool, Logger::DEBUG, "ICallback - Enter", g_debug_print);

    m_dispatcher->Register(this);

    LOG_WRITE(logger_pool, Logger::DEBUG, "ICallback - Exit", g_debug_print);
}

template <typename MSG>
ICallback<MSG>::~ICallback() noexcept
{
    LOG_WRITE(logger_pool, Logger::DEBUG, "~ICallback - Enter", g_debug_print);

    m_dispatcher->UnRegister(this);

    LOG_WRITE(logger_pool, Logger::DEBUG, "~ICallback - Exit", g_debug_print);
}


/**************** Callback Class ****************/

template <typename MSG, typename OBS>
class Callback : public ICallback<MSG>
{
public:
    using ActionMethod = void(OBS::*) (const MSG& msg_);
    using StopMethod = void(OBS::*) ();

    explicit Callback(Dispatcher<MSG>* dispatcher_, OBS& obj_, ActionMethod act_, StopMethod stop_ = nullptr);
    Callback(const Callback& other_) = delete;
    Callback& operator=(const Callback& other_) = delete;
    ~Callback() override = default;

private:
    void Notify(const MSG& msg_) override;
    void OutOfService() override; // Not Mandatory

    OBS& m_obj;
    ActionMethod m_act;
    StopMethod m_stop;
};


template <typename MSG, typename OBS>
Callback<MSG, OBS>::Callback(Dispatcher<MSG>* dispatcher_, OBS& obj_, ActionMethod act_, StopMethod stop_) :
ICallback<MSG>(dispatcher_), m_obj(obj_), m_act(act_), m_stop(stop_)
{
    LOG_WRITE(logger_pool, Logger::DEBUG, "Callback - Enter", g_debug_print);
}

template <typename MSG, typename OBS>
void Callback<MSG, OBS>::Notify(const MSG& msg_)
{
    LOG_WRITE(logger_pool, Logger::DEBUG, "Notify - Enter", g_debug_print);

    m_act(msg);

    LOG_WRITE(logger_pool, Logger::DEBUG, "Notify - Exit", g_debug_print);
}

template <typename MSG, typename OBS>
void Callback<MSG, OBS>::OutOfService()
{
    LOG_WRITE(logger_pool, Logger::DEBUG, "OutOfService - Enter", g_debug_print);

    nullptr != m_stop ? m_stop() :

    LOG_WRITE(logger_pool, Logger::DEBUG, "OutOfService - Exit", g_debug_print);
}

} // namespace hrd29

#endif /*_HRD29_EVENT_CHANNEL_HPP_*/

