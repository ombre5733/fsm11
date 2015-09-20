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
