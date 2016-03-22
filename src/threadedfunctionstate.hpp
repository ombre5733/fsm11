/*******************************************************************************
  fsm11 - A C++ library for finite state machines

  Copyright (c) 2015-2016, Manuel Freiberger
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

#ifndef FSM11_THREADEDFUNCTIONSTATE_HPP
#define FSM11_THREADEDFUNCTIONSTATE_HPP

#include "statemachine_fwd.hpp"
#include "exitrequest.hpp"
#include "functionstate.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/functional.hpp>
#include <weos/utility.hpp>
#include <weos/thread.hpp>
#else
#include <functional>
#include <utility>
#include <thread>
#endif // FSM11_USE_WEOS

namespace fsm11
{

struct invokeFunction_t {};
constexpr invokeFunction_t invokeFunction = invokeFunction_t();

template <typename TStateMachine>
class ThreadedFunctionState : public FunctionState<TStateMachine>
{
    using state_type = State<TStateMachine>;
    using base_type = FunctionState<TStateMachine>;
    using options = typename fsm11_detail::get_options<TStateMachine>::type;

public:
    using event_type = typename options::event_type;
    using function_type = FSM11STD::function<void(event_type)>;
    using invoke_function_type = FSM11STD::function<void(ExitRequest& exitRequest)>;
    using type = ThreadedFunctionState<TStateMachine>;

    explicit
    ThreadedFunctionState(const char* name, state_type* parent = nullptr)
        : base_type(name, parent)
    {
    }

    template <typename TEntry, typename TExit, typename TInvoke>
    ThreadedFunctionState(const char* name,
                          TEntry&& entryFn, TExit&& exitFn, TInvoke&& invokeFn,
                          state_type* parent = nullptr)
        : base_type(name,
                    FSM11STD::forward<TEntry>(entryFn),
                    FSM11STD::forward<TEntry>(exitFn),
                    parent),
          m_invokeFunction(FSM11STD::forward<TExit>(invokeFn))
    {
    }

    template <typename TEntry>
    ThreadedFunctionState(const char* name,
                          entryFunction_t, TEntry&& entryFn,
                          state_type* parent = nullptr)
        : base_type(name,
                    FSM11STD::forward<TEntry>(entryFn), nullptr,
                    parent)
    {
    }

    template <typename TExit>
    ThreadedFunctionState(const char* name,
                          exitFunction_t, TExit&& exitFn,
                          state_type* parent = nullptr)
        : base_type(name,
                    nullptr, FSM11STD::forward<TExit>(exitFn),
                    parent)
    {
    }

    template <typename TInvoke>
    ThreadedFunctionState(const char* name,
                          invokeFunction_t, TInvoke&& invokeFn,
                          state_type* parent = nullptr)
        : base_type(name, parent),
          m_invokeFunction(FSM11STD::forward<TInvoke>(invokeFn))
    {
    }

    template <typename TEntry, typename TExit>
    ThreadedFunctionState(const char* name,
                          entryFunction_t, TEntry&& entryFn,
                          exitFunction_t, TExit&& exitFn,
                          state_type* parent = nullptr)
        : base_type(name,
                    FSM11STD::forward<TEntry>(entryFn),
                    FSM11STD::forward<TEntry>(exitFn),
                    parent)
    {
    }

    template <typename TEntry, typename TInvoke>
    ThreadedFunctionState(const char* name,
                          entryFunction_t, TEntry&& entryFn,
                          invokeFunction_t, TInvoke&& invokeFn,
                          state_type* parent = nullptr)
        : base_type(name,
                    FSM11STD::forward<TEntry>(entryFn), nullptr,
                    parent),
          m_invokeFunction(FSM11STD::forward<TInvoke>(invokeFn))
    {
    }

    template <typename TExit, typename TInvoke>
    ThreadedFunctionState(const char* name,
                          exitFunction_t, TExit&& exitFn,
                          invokeFunction_t, TInvoke&& invokeFn,
                          state_type* parent = nullptr)
        : base_type(name,
                    nullptr, FSM11STD::forward<TExit>(exitFn),
                    parent),
          m_invokeFunction(FSM11STD::forward<TInvoke>(invokeFn))
    {
    }

    template <typename TEntry, typename TExit, typename TInvoke>
    ThreadedFunctionState(const char* name,
                          entryFunction_t, TEntry&& entryFn,
                          exitFunction_t, TExit&& exitFn,
                          invokeFunction_t, TInvoke&& invokeFn,
                          state_type* parent = nullptr)
        : base_type(name,
                    FSM11STD::forward<TEntry>(entryFn),
                    FSM11STD::forward<TEntry>(exitFn),
                    parent),
          m_invokeFunction(FSM11STD::forward<TExit>(invokeFn))
    {
    }

    ThreadedFunctionState(const ThreadedFunctionState&) = delete;
    ThreadedFunctionState& operator=(const ThreadedFunctionState&) = delete;

    //! Returns the invoke function.
    const invoke_function_type& invokeFunction() const
    {
        return m_invokeFunction;
    }

    //! \brief Sets the invoke function.
    //!
    //! Sets the invoke function to \p fn.
    template <typename T>
    void setInvokeFunction(T&& fn)
    {
        m_invokeFunction = FSM11STD::forward<T>(fn);
    }

    virtual
    void enterInvoke() override
    {
        m_exitRequest.m_mutex.lock();
        m_exitRequest.m_requested = false;
        m_exitRequest.m_mutex.unlock();

        m_exceptionPointer = nullptr;
        m_invokeThread = FSM11STD::thread(
#ifdef FSM11_USE_WEOS
                             m_invokeThreadAttributes,
#endif // FSM11_USE_WEOS
                             &ThreadedFunctionState::invokeWrapper, this);
    }

    virtual
    FSM11STD::exception_ptr exitInvoke() override
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
    weos::thread_attributes m_invokeThreadAttributes;
#endif // FSM11_USE_WEOS

    FSM11STD::thread m_invokeThread;
    FSM11STD::exception_ptr m_exceptionPointer;
    ExitRequest m_exitRequest;

    invoke_function_type m_invokeFunction;

    void invokeWrapper()
    {
        try
        {
            if (m_invokeFunction)
                m_invokeFunction(m_exitRequest);
        }
        catch (...)
        {
            m_exceptionPointer = FSM11STD::current_exception();
        }
    }
};

} // namespace fsm11

#endif // FSM11_THREADEDFUNCTIONSTATE_HPP
