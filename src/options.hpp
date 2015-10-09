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

#ifndef FSM11_OPTIONS_HPP
#define FSM11_OPTIONS_HPP

#include "statemachine_fwd.hpp"
#include "detail/options.hpp"

#include <deque>
#include <memory>

namespace fsm11
{

enum TransitionConflictPolicyEnum
{
    Ignore,
    InvokeCallback,
    ThrowException
};

namespace fsm11_detail
{

struct default_options
{
    // Types
    using event_type = int;
    using event_list_type = std::deque<int>;
    using capture_storage = type_list<>;
    using transition_allocator_type = std::allocator<Transition<void>>;

    // Behavior
    static constexpr bool synchronous_dispatch = true;
    static constexpr bool multithreading_enable = false;
    static constexpr TransitionConflictPolicyEnum transition_conflict_policy = Ignore;
    static constexpr bool transition_selection_stops_after_first_match = true;
    static constexpr bool threadpool_enable = false;

    // Callbacks
    static constexpr bool event_callbacks_enable = false;
    static constexpr bool configuration_change_callbacks_enable = false;
    static constexpr bool state_callbacks_enable = false;

    // Callbacks for exeptions
    static constexpr bool state_exception_callbacks_enable = false;
};

} // namespace fsm11_detail


// ----=====================================================================----
//     Types
// ----=====================================================================----

template <typename TType>
struct EventType
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        using event_type = TType;
    };
    //! \endcond
};

template <typename TType>
struct EventListType
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        using event_list_type = TType;
    };
    //! \endcond
};

template <typename... TTypes>
struct CaptureStorage
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        using capture_storage = fsm11_detail::type_list<TTypes...>;
    };
    //! \endcond
};

template <typename TAllocator>
struct TransitionAllocator
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        using transition_allocator_type = TAllocator;
    };
    //! \endcond
};

// ----=====================================================================----
//     Behaviour
// ----=====================================================================----

struct SynchronousEventDispatching
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool synchronous_dispatch = true;
    };
    //! \endcond
};

struct AsynchronousEventDispatching
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool synchronous_dispatch = false;
    };
    //! \endcond
};

template <bool TEnable>
struct MultithreadingEnable
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool multithreading_enable = TEnable;
    };
    //! \endcond
};

template <TransitionConflictPolicyEnum TPolicy>
struct TransitionConflictPolicy
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr TransitionConflictPolicyEnum transition_conflict_policy = TPolicy;
    };
    //! \endcond
};

template <bool TEnable>
struct TransitionSelectionStopsAfterFirstMatch
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool transition_selection_stops_after_first_match
                              = TEnable;
    };
    //! \endcond
};

template <bool TEnable, std::size_t... TNumPools>
struct ThreadPoolEnable;

template <>
struct ThreadPoolEnable<false>
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool threadpool_enable = false;
    };
    //! \endcond
};

template <std::size_t TNumPools>
struct ThreadPoolEnable<true, TNumPools>
{
    static_assert(TNumPools > 0, "The pool size must be non-zero.");

    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool threadpool_enable = true;
        static constexpr std::size_t thread_pool_size = TNumPools;
    };
    //! \endcond
};

// ----=====================================================================----
//     Callbacks
// ----=====================================================================----

template <bool TEnable>
struct EventCallbacksEnable
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool event_callbacks_enable = TEnable;
    };
    //! \endcond
};

template <bool TEnable>
struct ConfigurationChangeCallbacksEnable
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool configuration_change_callbacks_enable = TEnable;
    };
    //! \endcond
};

template <bool TEnable>
struct StateCallbacksEnable
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool state_callbacks_enable = TEnable;
    };
    //! \endcond
};

// ----=====================================================================----
//     Exception callbacks
// ----=====================================================================----

template <bool TEnable>
struct StateExceptionCallbacksEnable
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool state_exception_callbacks_enable = TEnable;
    };
    //! \endcond
};

} // namespace fsm11

#endif // FSM11_OPTIONS_HPP
