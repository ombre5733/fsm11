/*******************************************************************************
  The MIT License (MIT)

  Copyright (c) 2015 Manuel Freiberger

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*******************************************************************************/

#ifndef SCOPEGUARD_HPP
#define SCOPEGUARD_HPP

#include "../statemachine_fwd.hpp"

#ifdef FSM11_USE_WEOS

#include <weos/scopeguard.hpp>

#define FSM11_SCOPE_EXIT      WEOS_SCOPE_EXIT
#define FSM11_SCOPE_FAILURE   WEOS_SCOPE_FAILURE
#define FSM11_SCOPE_SUCCESS   WEOS_SCOPE_SUCCESS

#else // FSM11_USE_WEOS

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

#else

#error "Unknown compiler."

#endif // __GNUC__

namespace fsm11
{
namespace fsm11_detail
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

    ~ScopeGuard()
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

} // namespace fsm11_detail
} // namespace fsm11

// Helper macros for the generation of anonymous variables.
#define FSM11_CONCATENATE_HELPER(a, b)   a ## b
#define FSM11_CONCATENATE(a, b)          FSM11_CONCATENATE_HELPER(a, b)
#define FSM11_ANONYMOUS_VARIABLE(name)   FSM11_CONCATENATE(name, __LINE__)


#define FSM11_SCOPE_EXIT                                                       \
    auto FSM11_ANONYMOUS_VARIABLE(_fsm11_scopeGuard_) =                        \
    ::fsm11::fsm11_detail::OnScopeExit() + [&]() noexcept

#define FSM11_SCOPE_FAILURE                                                    \
    auto FSM11_ANONYMOUS_VARIABLE(_fsm11_scopeGuard_) =                        \
    ::fsm11::fsm11_detail::OnScopeFailure() + [&]() noexcept

#define FSM11_SCOPE_SUCCESS                                                    \
    auto FSM11_ANONYMOUS_VARIABLE(_fsm11_scopeGuard_) =                        \
    ::fsm11::fsm11_detail::OnScopeSuccess() + [&]()

#endif // FSM11_USE_WEOS

#endif // SCOPEGUARD_HPP
