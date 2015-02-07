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

namespace std
{
inline
int uncaught_exceptions() noexcept
{
    return *reinterpret_cast<int*>(static_cast<char*>(static_cast<void*>(__cxxabiv1::__cxa_get_globals())) + sizeof(void*));
}
} // namespace std

#endif // __GNUC__

namespace weos_exception_detail
{

template <typename TCallable>
class ScopeGuard
{
public:
    template <typename T,
              typename _ = typename std::enable_if<!std::is_same<typename std::decay<T>::type,
                                                                 ScopeGuard>::value>::type>
    explicit ScopeGuard(T&& callable) // TODO: noexcept
        : m_callable(callable),
          m_dismissed(false)
    {
    }

    ScopeGuard(ScopeGuard&& other) // TODO: noexcept
        : m_callable(std::move(other.m_callable)),
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
              typename _ = typename std::enable_if<!std::is_same<typename std::decay<T>::type,
                                                                 ExceptionScopeGuard>::value>::type>
    explicit ExceptionScopeGuard(T&& callable) // TODO: noexcept
        : m_callable(std::forward<T>(callable)),
          m_numExceptions(std::uncaught_exceptions())
    {
    }

    ExceptionScopeGuard(ExceptionScopeGuard&& other) // TODO: noexcept
        : m_callable(std::move(other.m_callable)),
          m_numExceptions(other.m_numExceptions)
    {
    }

    ~ExceptionScopeGuard() noexcept(TExecuteOnException)
    {
        if (TExecuteOnException
            == (std::uncaught_exceptions() > m_numExceptions))
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
ScopeGuard<typename std::decay<TCallable>::type> operator+(
        OnScopeExit, TCallable&& callable)
{
    return ScopeGuard<typename std::decay<TCallable>::type>(
                std::forward<TCallable>(callable));
}

template <typename TCallable>
ExceptionScopeGuard<typename std::decay<TCallable>::type, true> operator+(
        OnScopeFailure, TCallable&& callable)
{
    return ExceptionScopeGuard<typename std::decay<TCallable>::type, true>(
                std::forward<TCallable>(callable));
}

template <typename TCallable>
ExceptionScopeGuard<typename std::decay<TCallable>::type, false> operator+(
        OnScopeSuccess, TCallable&& callable)
{
    return ExceptionScopeGuard<typename std::decay<TCallable>::type, false>(
                std::forward<TCallable>(callable));
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
