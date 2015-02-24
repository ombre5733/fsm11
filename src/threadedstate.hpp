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

#ifdef FSM11_USE_WEOS
#include <weos/exception.hpp>
#include <weos/thread.hpp>
#else
#include <exception>
#include <thread>
#endif // FSM11_USE_WEOS

namespace fsm11
{

//! \brief A state with a threaded invoke action.
template <typename TStateMachine>
class ThreadedState : public State<TStateMachine>
{
    using base_type = State<TStateMachine>;

public:
    using type = ThreadedState<TStateMachine>;

#ifdef FSM11_USE_WEOS
    //! \brief Creates a state with a threaded invoke action.
    explicit ThreadedState(const char* name,
                           const weos::thread::attributes& attrs,
                           base_type* parent = 0)
        : base_type(name, parent),
          m_invokeThreadAttributes(attrs)
    {
    }
#else
    //! \brief Creates a state with a threaded invoke action.
    explicit ThreadedState(const char* name, base_type* parent = 0)
        : base_type(name, parent)
    {
    }
#endif // FSM11_USE_WEOS

    virtual ~ThreadedState()
    {
        // If the invoked thread is joinable, exitInvoke() has not been
        // called. This is the case if the state is destructed before the
        // state machine is stopped.
        FSM11_ASSERT(!m_invokeThread.joinable());
    }

    //! \brief The actual invoke action.
    //!
    //! This method is called in a new thread. Derived classes have to
    //! provide an implementation.
    virtual void invoke(ExitRequest& exitRequest) = 0;

    //! Enteres the invoked thread.
    virtual void enterInvoke() override
    {
        m_exitRequest.m_mutex.lock();
        m_exitRequest.m_requested = false;
        m_exitRequest.m_mutex.unlock();

        m_exceptionPointer = nullptr;
        m_invokeThread = FSM11STD::thread(
#ifdef FSM11_USE_WEOS
                             m_invokeThreadAttributes,
#endif // FSM11_USE_WEOS
                             &ThreadedState::invokeWrapper, this);
    }

    //! Leaves the invoked thread.
    //!
    //! Joins with the thread in which the invoked action is running.
    virtual FSM11STD::exception_ptr exitInvoke() override
    {
        FSM11_ASSERT(m_invokeThread.joinable());

        m_exitRequest.m_mutex.lock();
        m_exitRequest.m_requested = true;
        m_exitRequest.m_mutex.unlock();
        m_exitRequest.m_cv.notify_one();

        m_invokeThread.join();
        return m_exceptionPointer;
    }

private:
#ifdef FSM11_USE_WEOS
    weos::thread::attributes m_invokeThreadAttributes;
#endif // FSM11_USE_WEOS

    FSM11STD::thread m_invokeThread;
    FSM11STD::exception_ptr m_exceptionPointer;

    ExitRequest m_exitRequest;

    void invokeWrapper()
    {
        try
        {
            invoke(m_exitRequest);
        }
        catch (...)
        {
            m_exceptionPointer = FSM11STD::current_exception();
        }
    }
};

} // namespace fsm11

#endif // FSM11_THREADEDSTATE_HPP
