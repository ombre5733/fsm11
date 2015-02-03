#ifndef FSM11_DETAIL_MULTITHREADING_HPP
#define FSM11_DETAIL_MULTITHREADING_HPP

#include "statemachine_fwd.hpp"

#include <mutex>
#include <type_traits>

namespace fsm11
{
namespace detail
{

class WithoutMultithreading
{
public:
    template <typename T = void>
    void lock()
    {
        static_assert(!std::is_same<T, T>::value,
                      "Multithreading support is not enabled");
    }

    template <typename T = void>
    void unlock()
    {
        static_assert(!std::is_same<T, T>::value,
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
    mutable std::recursive_mutex m_mutex;

    inline
    std::unique_lock<std::recursive_mutex> getLock() const
    {
        return std::unique_lock<std::recursive_mutex>(m_mutex);
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

} // namespace detail
} // namespace fsm11

#endif // FSM11_DETAIL_MULTITHREADING_HPP
