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

#ifndef FSM11_FUNCTIONSTATE_HPP
#define FSM11_FUNCTIONSTATE_HPP

#include "state.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/functional.hpp>
#include <weos/utility.hpp>
#else
#include <functional>
#include <utility>
#endif // FSM11_USE_WEOS

namespace fsm11
{

struct entryFunction_t {};
constexpr entryFunction_t entryFunction = entryFunction_t();

struct exitFunction_t {};
constexpr exitFunction_t exitFunction = exitFunction_t();

template <typename TStateMachine>
class FunctionState : public State<TStateMachine>
{
    using base_type = State<TStateMachine>;
    using options = typename fsm11_detail::get_options<TStateMachine>::type;

public:
    using event_type = typename options::event_type;
    using function_type = FSM11STD::function<void(event_type)>;
    using type = FunctionState<TStateMachine>;

    explicit FunctionState(const char* name, base_type* parent = nullptr)
        : base_type(name, parent)
    {
    }

    template <typename TEntry, typename TExit>
    FunctionState(const char* name,
                  TEntry&& entryFn, TExit&& exitFn,
                  base_type* parent = nullptr)
        : base_type(name, parent),
          m_entryFunction(FSM11STD::forward<TEntry>(entryFn)),
          m_exitFunction(FSM11STD::forward<TExit>(exitFn))
    {
    }

    template <typename TEntry>
    FunctionState(const char* name,
                  entryFunction_t, TEntry&& fn,
                  base_type* parent = nullptr)
        : base_type(name, parent),
          m_entryFunction(FSM11STD::forward<TEntry>(fn))
    {
    }

    template <typename TExit>
    FunctionState(const char* name,
                  exitFunction_t, TExit&& fn,
                  base_type* parent = nullptr)
        : base_type(name, parent),
          m_exitFunction(FSM11STD::forward<TExit>(fn))
    {
    }

    template <typename TEntry, typename TExit>
    FunctionState(const char* name,
                  entryFunction_t, TEntry&& entryFn,
                  exitFunction_t, TExit&& exitFn,
                  base_type* parent = nullptr)
        : base_type(name, parent),
          m_entryFunction(FSM11STD::forward<TEntry>(entryFn)),
          m_exitFunction(FSM11STD::forward<TExit>(exitFn))
    {
    }

    FunctionState(const FunctionState&) = delete;
    FunctionState& operator=(const FunctionState&) = delete;

    //! Returns the entry function.
    const function_type& entryFunction() const
    {
        return m_entryFunction;
    }

    //! \brief Sets the entry function.
    //!
    //! Sets the entry function to \p fn.
    template <typename T>
    void setEntryFunction(T&& fn)
    {
        m_entryFunction = FSM11STD::forward<T>(fn);
    }

    //! Returns the exit function.
    const function_type& exitFunction() const
    {
        return m_exitFunction;
    }

    //! \brief Sets the exit function.
    //!
    //! Sets the exit function to \p fn.
    template <typename T>
    void setExitFunction(T&& fn)
    {
        m_exitFunction = FSM11STD::forward<T>(fn);
    }

    virtual void onEntry(event_type event) override
    {
        if (m_entryFunction)
            m_entryFunction(event);
    }

    virtual void onExit(event_type event) override
    {
        if (m_exitFunction)
            m_exitFunction(event);
    }

private:
    function_type m_entryFunction;
    function_type m_exitFunction;
};

} // namespace fsm11

#endif // FSM11_FUNCTIONSTATE_HPP
