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
class ShallowHistoryState;

template <typename TStateMachine>
class DeepHistoryState;

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
