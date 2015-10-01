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

#ifndef FSM11_OPTIONS_HPP
#define FSM11_OPTIONS_HPP

#include "statemachine_fwd.hpp"
#include "detail/options.hpp"

#include <deque>
#include <memory>

namespace fsm11
{
namespace fsm11_detail
{

struct default_options
{
    // Types
    using event_type = int;
    using event_list_type = std::deque<int>;
    using capture_storage = type_list<>;
    using transition_allocator_type = std::allocator<Transition<void>>;

    // Behaviour
    static constexpr bool synchronous_dispatch = true;
    static constexpr bool multithreading_enable = false;
    static constexpr bool transition_selection_stops_after_first_match = true;
    static constexpr bool threadpool_enable = false;

    // Callbacks
    static constexpr bool event_callbacks_enable = false;
    static constexpr bool configuration_change_callbacks_enable = false;
    static constexpr bool state_callbacks_enable = false;
    static constexpr bool transition_conflict_callbacks_enable = false;

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

template <bool TEnable>
struct TransitionConflictCallbacksEnable
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool transition_conflict_callbacks_enable = TEnable;
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
