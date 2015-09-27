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

#ifndef FSM11_THREADEDSTATE_HPP
#define FSM11_THREADEDSTATE_HPP

#include "statemachine_fwd.hpp"
#include "exitrequest.hpp"
#include "state.hpp"
#include "detail/threadedstatebase.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/future.hpp>
#include <weos/thread.hpp>
#else
#include <future>
#endif // FSM11_USE_WEOS

namespace fsm11
{

//! \brief A state with a threaded invoke action.
template <typename TStateMachine>
class ThreadedState : public State<TStateMachine>,
                      public fsm11_detail::ThreadedStateBase
{
    using base_type = State<TStateMachine>;

public:
    using type = ThreadedState<TStateMachine>;

#ifdef FSM11_USE_WEOS
    //! \brief Creates a state with a threaded invoke action.
    explicit ThreadedState(const char* name,
                           const weos::thread::attributes& attrs,
                           base_type* parent = nullptr)
        : base_type(name, parent),
          m_invokeThreadAttributes(attrs)
    {
    }
#else
    //! \brief Creates a state with a threaded invoke action.
    explicit ThreadedState(const char* name, base_type* parent = nullptr)
        : base_type(name, parent)
    {
    }
#endif // FSM11_USE_WEOS

    //! \brief The actual invoke action.
    //!
    //! This method is called in a new thread. Derived classes have to
    //! provide an implementation.
    virtual void invoke(ExitRequest& exitRequest) = 0;

    //! Enters the invoked thread.
    virtual void enterInvoke() override final
    {
        m_exitRequest.m_mutex.lock();
        m_exitRequest.m_requested = false;
        m_exitRequest.m_mutex.unlock();

#ifdef FSM11_USE_WEOS
        m_result = weos::async(m_invokeThreadAttributes,
                               &ThreadedState::invoke, this,
                               weos::ref(m_exitRequest));
#else
        m_result = std::async(std::launch::async,
                              &ThreadedState::invoke, this,
                              std::ref(m_exitRequest));
#endif // FSM11_USE_WEOS
    }

    //! Leaves the invoked thread.
    //!
    //! Joins with the thread in which the invoked action is running.
    virtual void exitInvoke() override final
    {
        m_exitRequest.m_mutex.lock();
        m_exitRequest.m_requested = true;
        m_exitRequest.m_mutex.unlock();
        m_exitRequest.m_cv.notify_one();

        m_result.get();
    }

private:
#ifdef FSM11_USE_WEOS
    weos::thread::attributes m_invokeThreadAttributes;
#endif // FSM11_USE_WEOS

    FSM11STD::future<void> m_result;
};

} // namespace fsm11

#endif // FSM11_THREADEDSTATE_HPP
