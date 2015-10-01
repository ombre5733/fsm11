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
#include <weos/tuple.hpp>
#else
#include <future>
#include <tuple>
#endif // FSM11_USE_WEOS

namespace fsm11
{

//! \brief A state with a threaded invoke action.
template <typename TStateMachine>
class ThreadedState : public State<TStateMachine>,
                      public fsm11_detail::ThreadedStateBase
{
    using base_type = State<TStateMachine>;

    static constexpr bool has_thread_pool
        = fsm11_detail::get_options<TStateMachine>::type::threadpool_enable;

public:
    using type = ThreadedState<TStateMachine>;

#ifdef FSM11_USE_WEOS
    //! \brief Creates a state with a threaded invoke action.
    template <typename T = void,
              typename = typename FSM11STD::enable_if<
                             has_thread_pool, T>::type>
    explicit ThreadedState(const char* name, base_type* parent = nullptr)
        : base_type(name, parent)
    {
    }

    //! \brief Creates a state with a threaded invoke action.
    template <typename T = void,
              typename = typename FSM11STD::enable_if<
                             !has_thread_pool, T>::type>
    explicit ThreadedState(const char* name,
                           const FSM11STD::thread::attributes& attrs,
                           base_type* parent = nullptr)
        : base_type(name, parent)
    {
        FSM11STD::get<1>(m_data) = attrs;
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

        doEnterInvoke(FSM11STD::integral_constant<bool, has_thread_pool>());
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

        FSM11STD::get<0>(m_data).get();
    }

private:
#ifdef FSM11_USE_WEOS
    struct None {};

    using maybe_thread_attributes_t
        = typename FSM11STD::conditional<has_thread_pool,
                                         FSM11STD::thread::attributes,
                                         None>::type;

    using data_type = FSM11STD::tuple<FSM11STD::future<void>,
                                      maybe_thread_attributes_t>;
#else
    using data_type = FSM11STD::tuple<FSM11STD::future<void>>;
#endif // FSM11_USE_WEOS

    data_type m_data;


    void doEnterInvoke(FSM11STD::false_type)
    {
        using namespace FSM11STD;

        get<0>(m_data) = async(launch::async,
#ifdef FSM11_USE_WEOS
                               get<1>(m_data),
#endif // FSM11_USE_WEOS
                               &ThreadedStateBase::invoke,
                               this, ref(this->m_exitRequest));
    }

    void doEnterInvoke(FSM11STD::true_type)
    {
        FSM11STD::get<0>(m_data)
                = this->stateMachine()->threadPool().enqueue(*this);
    }
};

} // namespace fsm11

#endif // FSM11_THREADEDSTATE_HPP
