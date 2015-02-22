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

template <typename TStateMachine>
class FunctionState : public State<TStateMachine>
{
    using base_type = State<TStateMachine>;
    using options = typename fsm11_detail::get_options<TStateMachine>::type;

public:
    using event_type = typename options::event_type;
    using function_type = FSM11STD::function<void(event_type)>;
    using type = FunctionState<TStateMachine>;

    explicit FunctionState(const char* name, base_type* parent = 0)
        : base_type(name, parent)
    {
    }

    template <typename TEntry, typename TExit>
    FunctionState(const char* name,
                  TEntry&& entryFn, TExit&& exitFn,
                  base_type* parent = 0)
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
