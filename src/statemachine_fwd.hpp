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

#ifndef FSM11_STATEMACHINE_FWD_HPP
#define FSM11_STATEMACHINE_FWD_HPP

// If the macro FSM11_USER_CONFIG is set, it points to the user's
// configuration file. If the macro is not set, we assume that the
// user configuration is somewhere in the path.
#if defined(FSM11_USER_CONFIG)
    #include FSM11_USER_CONFIG
#else
    #include "fsm11_user_config.hpp"
#endif // FSM11_USER_CONFIG

// ----=====================================================================----
//     Assertion handling
// ----=====================================================================----

#if defined(FSM11_ENABLE_ASSERT)
    #include <cassert>
    #define FSM11_ASSERT(cond)   assert(cond)
#else
    #define FSM11_ASSERT(cond)   ((void)0)
#endif // FSM11_ENABLE_ASSERT

// ----=====================================================================----
//     WEOS integration
// ----=====================================================================----

#ifdef FSM11_USE_WEOS
    #define FSM11STD   weos
    #define FSM11_EXCEPTION(x)   WEOS_EXCEPTION(x)
#else
    #define FSM11STD   std
    #define FSM11_EXCEPTION(x)   x
#endif // FSM11_USE_WEOS


#include <cstddef>

namespace fsm11
{

template <typename TStateMachine>
class HistoryState;

template <typename TStateMachine>
class State;

template <typename TStateMachine>
class ThreadedState;

template <typename TStateMachine>
class ThreadedFunctionState;

template <std::size_t TSize>
class ThreadPool;

template <typename TStateMachine>
class Transition;

namespace fsm11_detail
{

template <typename TDerived>
class EventDispatcherBase;

template <typename TOptions>
class StateMachineImpl;


template <typename TType>
struct get_options;

template <typename TOptions>
struct get_options<StateMachineImpl<TOptions>>
{
    typedef TOptions type;
};

} // namespace fsm11_detail

template <typename TStateMachine>
State<TStateMachine>* findLeastCommonProperAncestor(
        State<TStateMachine>* state1, State<TStateMachine>* state2) noexcept;

} // namespace fsm11

#endif // FSM11_STATEMACHINE_FWD_HPP
