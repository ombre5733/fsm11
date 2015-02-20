#ifndef SCOPEGUARD_HPP
#define SCOPEGUARD_HPP

#include <exception>
#include <utility>
#include <type_traits>

#if __GNUC__

struct __cxa_eh_globals;

namespace __cxxabiv1
{
extern "C" __cxa_eh_globals* __cxa_get_globals();
} // namespace __cxxabiv1

namespace FSM11STD
{
inline
int uncaught_exceptions() noexcept
{
    return *reinterpret_cast<int*>(static_cast<char*>(static_cast<void*>(__cxxabiv1::__cxa_get_globals())) + sizeof(void*));
}
} // namespace FSM11STD

#endif // __GNUC__

namespace weos_exception_detail
{

template <typename TCallable>
class ScopeGuard
{
public:
    template <typename T,
              typename _ = typename FSM11STD::enable_if<!FSM11STD::is_same<typename FSM11STD::decay<T>::type,
                                                                           ScopeGuard>::value>::type>
    explicit ScopeGuard(T&& callable) // TODO: noexcept
        : m_callable(callable),
          m_dismissed(false)
    {
    }

    ScopeGuard(ScopeGuard&& other) // TODO: noexcept
        : m_callable(FSM11STD::move(other.m_callable)),
          m_dismissed(other.m_dismissed)
    {
        other.m_dismissed = true;
    }

    ~ScopeGuard() noexcept
    {
        if (!m_dismissed)
        {
            m_callable();
        }
    }

    ScopeGuard(const ScopeGuard& other) = delete;

    void dismiss() noexcept
    {
        m_dismissed = true;
    }

private:
    TCallable m_callable;
    bool m_dismissed;
};

template <typename TCallable, bool TExecuteOnException>
class ExceptionScopeGuard
{
public:
    template <typename T,
              typename _ = typename FSM11STD::enable_if<!FSM11STD::is_same<typename FSM11STD::decay<T>::type,
                                                                           ExceptionScopeGuard>::value>::type>
    explicit ExceptionScopeGuard(T&& callable) // TODO: noexcept
        : m_callable(FSM11STD::forward<T>(callable)),
          m_numExceptions(FSM11STD::uncaught_exceptions())
    {
    }

    ExceptionScopeGuard(ExceptionScopeGuard&& other) // TODO: noexcept
        : m_callable(FSM11STD::move(other.m_callable)),
          m_numExceptions(other.m_numExceptions)
    {
    }

    ~ExceptionScopeGuard() noexcept(TExecuteOnException)
    {
        if (TExecuteOnException
            == (FSM11STD::uncaught_exceptions() > m_numExceptions))
        {
            m_callable();
        }
    }

    ExceptionScopeGuard(const ExceptionScopeGuard& other) = delete;

private:
    TCallable m_callable;
    int m_numExceptions;
};

struct OnScopeExit {};
struct OnScopeFailure {};
struct OnScopeSuccess {};

template <typename TCallable>
ScopeGuard<typename FSM11STD::decay<TCallable>::type> operator+(
        OnScopeExit, TCallable&& callable)
{
    return ScopeGuard<typename FSM11STD::decay<TCallable>::type>(
                FSM11STD::forward<TCallable>(callable));
}

template <typename TCallable>
ExceptionScopeGuard<typename FSM11STD::decay<TCallable>::type, true> operator+(
        OnScopeFailure, TCallable&& callable)
{
    return ExceptionScopeGuard<typename FSM11STD::decay<TCallable>::type, true>(
                FSM11STD::forward<TCallable>(callable));
}

template <typename TCallable>
ExceptionScopeGuard<typename FSM11STD::decay<TCallable>::type, false> operator+(
        OnScopeSuccess, TCallable&& callable)
{
    return ExceptionScopeGuard<typename FSM11STD::decay<TCallable>::type, false>(
                FSM11STD::forward<TCallable>(callable));
}

} // namespace weos_exception_detail


// Helper macros for the generation of anonymous variables.
#define WEOS_CONCATENATE_HELPER(a, b)   a ## b
#define WEOS_CONCATENATE(a, b)          WEOS_CONCATENATE_HELPER(a, b)
#define WEOS_ANONYMOUS_VARIABLE(name)   WEOS_CONCATENATE(name, __LINE__)


#define SCOPE_EXIT \
    auto WEOS_ANONYMOUS_VARIABLE(_weos_scopeGuard_) = \
    weos_exception_detail::OnScopeExit() + [&]() noexcept

#define SCOPE_FAILURE \
    auto WEOS_ANONYMOUS_VARIABLE(_weos_scopeGuard_) = \
    weos_exception_detail::OnScopeFailure() + [&]() noexcept

#define SCOPE_SUCCESS \
    auto WEOS_ANONYMOUS_VARIABLE(_weos_scopeGuard_) = \
    weos_exception_detail::OnScopeSuccess() + [&]() noexcept

#endif // SCOPEGUARD_HPP
