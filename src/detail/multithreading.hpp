/*******************************************************************************
  fsm11 - A C++11-compliant framework for finite state machines

  Copyright (c) 2015, Manuel Freiberger
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef FSM11_DETAIL_MULTITHREADING_HPP
#define FSM11_DETAIL_MULTITHREADING_HPP

#include "../statemachine_fwd.hpp"

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

template <typename TOptions>
struct get_multithreading
{
    using type = typename FSM11STD::conditional<
                     TOptions::multithreading_enable,
                     WithMultithreading,
                     WithoutMultithreading>::type;
};

} // namespace fsm11_detail
} // namespace fsm11

#endif // FSM11_DETAIL_MULTITHREADING_HPP
