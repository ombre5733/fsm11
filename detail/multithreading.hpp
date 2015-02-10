#ifndef FSM11_DETAIL_MULTITHREADING_HPP
#define FSM11_DETAIL_MULTITHREADING_HPP

#include "statemachine_fwd.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/mutex.hpp>
#include <weos/type_traits.hpp>
#else
#include <mutex>
#include <type_traits>
#endif // FSM11_USE_WEOS

namespace fsm11
{
namespace fsm11_detail
{

class WithoutMultithreading
{
public:
    template <typename T = void>
    void lock()
    {
        static_assert(!FSM11STD::is_same<T, T>::value,
                      "Multithreading support is not enabled");
    }

    template <typename T = void>
    void unlock()
    {
        static_assert(!FSM11STD::is_same<T, T>::value,
                      "Multithreading support is not enabled");
    }

protected:
    inline
    int getLock() const
    {
        return 0;
    }
};

class WithMultithreading
{
public:
    void lock()
    {
        m_mutex.lock();
    }

    void unlock()
    {
        m_mutex.unlock();
    }

protected:
    mutable FSM11STD::recursive_mutex m_mutex;

    inline
    FSM11STD::unique_lock<FSM11STD::recursive_mutex> getLock() const
    {
        return FSM11STD::unique_lock<FSM11STD::recursive_mutex>(m_mutex);
    }
};

template <bool TEnabled>
struct get_multithreading_helper
{
    typedef WithoutMultithreading type;
};

template <>
struct get_multithreading_helper<true>
{
    typedef WithMultithreading type;
};

template <typename TOptions>
struct get_multithreading : public get_multithreading_helper<TOptions::multithreading_enable>
{
};

} // namespace fsm11_detail
} // namespace fsm11

#endif // FSM11_DETAIL_MULTITHREADING_HPP
