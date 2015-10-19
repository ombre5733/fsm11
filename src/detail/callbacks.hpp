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

#ifndef FSM11_DETAIL_CALLBACKS_HPP
#define FSM11_DETAIL_CALLBACKS_HPP

#include "../statemachine_fwd.hpp"
#include "../error.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/functional.hpp>
#include <weos/utility.hpp>
#include <weos/type_traits.hpp>
#else
#include <functional>
#include <utility>
#include <type_traits>
#endif // FSM11_USE_WEOS

namespace fsm11
{
namespace fsm11_detail
{
// ----=====================================================================----
//     Event callbacks
// ----=====================================================================----

template <typename TDerived>
class WithoutEventCallbacks
{
public:
    using options = typename get_options<TDerived>::type;
    using event_type = typename options::event_type;

    template <typename TType>
    void setEventDispatchCallback(TType&&)
    {
        static_assert(!FSM11STD::is_same<TType, TType>::value,
                      "Event callbacks are disabled");
    }

    template <typename TType>
    void setEventDiscardedCallback(TType&&)
    {
        static_assert(!FSM11STD::is_same<TType, TType>::value,
                      "Event callbacks are disabled");
    }

protected:
    inline
    void invokeEventDispatchCallback(event_type)
    {
    }

    inline
    void invokeEventDiscardedCallback(event_type)
    {
    }
};

template <typename TDerived>
class WithEventCallbacks
{
public:
    using options = typename get_options<TDerived>::type;
    using event_type = typename options::event_type;

    template <typename TType>
    void setEventDispatchCallback(TType&& callback)
    {
        m_eventDispatchCallback = FSM11STD::forward<TType>(callback);
    }

    template <typename TType>
    void setEventDiscardedCallback(TType&& callback)
    {
        m_eventDiscardedCallback = FSM11STD::forward<TType>(callback);
    }

protected:
    inline
    void invokeEventDispatchCallback(event_type event)
    {
        if (m_eventDispatchCallback)
            m_eventDispatchCallback(event);
    }

    inline
    void invokeEventDiscardedCallback(event_type event)
    {
        if (m_eventDiscardedCallback)
            m_eventDiscardedCallback(event);
    }

private:
    FSM11STD::function<void(event_type)> m_eventDispatchCallback;
    FSM11STD::function<void(event_type)> m_eventDiscardedCallback;
};

template <bool TEnabled, typename TOptions>
struct get_event_callbacks_helper
{
    typedef WithoutEventCallbacks<StateMachineImpl<TOptions>> type;
};

template <typename TOptions>
struct get_event_callbacks_helper<true, TOptions>
{
    typedef WithEventCallbacks<StateMachineImpl<TOptions>> type;
};

template <typename TOptions>
struct get_event_callbacks : public get_event_callbacks_helper<TOptions::event_callbacks_enable, TOptions>
{
};

// ----=====================================================================----
//     Configuration change callback
// ----=====================================================================----

class WithoutConfigurationChangeCallback
{
public:
    template <typename TType>
    void setConfigurationChangeCallback(TType&&)
    {
        static_assert(!FSM11STD::is_same<TType, TType>::value,
                      "Configuration change callbacks are disabled");
    }

protected:
    inline
    void invokeConfigurationChangeCallback()
    {
    }
};

class WithConfigurationChangeCallback
{
public:
    template <typename TType>
    void setConfigurationChangeCallback(TType&& callback)
    {
        m_configurationChangeCallback = FSM11STD::forward<TType>(callback);
    }

protected:
    inline
    void invokeConfigurationChangeCallback()
    {
        if (m_configurationChangeCallback)
            m_configurationChangeCallback();
    }

private:
    FSM11STD::function<void()> m_configurationChangeCallback;
};

template <typename TOptions>
struct get_configuration_change_callbacks
{
    using type = typename FSM11STD::conditional<
                     TOptions::configuration_change_callbacks_enable,
                     WithConfigurationChangeCallback,
                     WithoutConfigurationChangeCallback>::type;
};

// ----=====================================================================----
//     State callbacks
// ----=====================================================================----

template <typename TDerived>
class WithoutStateCallbacks
{
public:
    using state_type = State<TDerived>;

    template <typename TType>
    void setStateEntryCallback(TType&&)
    {
        static_assert(!FSM11STD::is_same<TType, TType>::value,
                      "State callbacks are disabled");
    }

    template <typename TType>
    void setStateExitCallback(TType&&)
    {
        static_assert(!FSM11STD::is_same<TType, TType>::value,
                      "State callbacks are disabled");
    }

protected:
    inline
    void invokeStateEntryCallback(state_type*)
    {
    }

    inline
    void invokeStateExitCallback(state_type*)
    {
    }
};

template <typename TDerived>
class WithStateCallbacks
{
public:
    using state_type = State<TDerived>;

    template <typename TType>
    void setStateEntryCallback(TType&& callback)
    {
        m_stateEntryCallback = FSM11STD::forward<TType>(callback);
    }

    template <typename TType>
    void setStateExitCallback(TType&& callback)
    {
        m_stateExitCallback = FSM11STD::forward<TType>(callback);
    }

protected:
    inline
    void invokeStateEntryCallback(state_type* state)
    {
        if (m_stateEntryCallback)
            m_stateEntryCallback(state);
    }

    inline
    void invokeStateExitCallback(state_type* state)
    {
        if (m_stateExitCallback)
            m_stateExitCallback(state);
    }

private:
    FSM11STD::function<void(state_type*)> m_stateEntryCallback;
    FSM11STD::function<void(state_type*)> m_stateExitCallback;
};

template <typename TOptions>
struct get_state_callbacks
{
    using type = typename FSM11STD::conditional<
                     TOptions::state_callbacks_enable,
                     WithStateCallbacks<StateMachineImpl<TOptions>>,
                     WithoutStateCallbacks<StateMachineImpl<TOptions>>>::type;
};

// ----=====================================================================----
//     State exception callbacks
// ----=====================================================================----

class WithoutStateExceptionCallbacks
{
public:
    template <typename TType>
    void setStateExceptionCallback(TType&&)
    {
        static_assert(!FSM11STD::is_same<TType, TType>::value,
                      "State exception callbacks are disabled");
    }

protected:
    inline
    void invokeStateExceptionCallbackOrThrow()
    {
        throw;
    }
};

template <typename TDerived>
class WithStateExceptionCallbacks
{
public:
    template <typename TType>
    void setStateExceptionCallback(TType&& callback)
    {
        m_stateExceptionCallback = FSM11STD::forward<TType>(callback);
    }

protected:
    inline
    void invokeStateExceptionCallbackOrThrow()
    {
        if (m_stateExceptionCallback)
            m_stateExceptionCallback(FSM11STD::current_exception());
    }

private:
    FSM11STD::function<void(FSM11STD::exception_ptr)> m_stateExceptionCallback;
};

template <typename TOptions>
struct get_state_exception_callbacks
{
    using type = typename FSM11STD::conditional<
                     TOptions::state_exception_callbacks_enable,
                     WithStateExceptionCallbacks<StateMachineImpl<TOptions>>,
                     WithoutStateExceptionCallbacks>::type;
};

// ----=====================================================================----
//     Transition conflict callback
// ----=====================================================================----

template <typename TDerived>
class WithoutTransitionConflictAction
{
public:
    using transition_type = Transition<TDerived>;

    template <typename TType>
    void setTransitionConflictCallback(TType&&)
    {
        static_assert(!FSM11STD::is_same<TType, TType>::value,
                      "Transition conflict callbacks are disabled");
    }

protected:
    inline
    void invokeTransitionConflictAction(transition_type*, transition_type*)
    {
    }

    inline
    bool hasTransitionConflictAction() const
    {
        return false;
    }
};

template <typename TDerived>
class WithTransitionConflictCallback
{
public:
    using transition_type = Transition<TDerived>;

    template <typename TType>
    void setTransitionConflictCallback(TType&& callback)
    {
        m_transitionConflictCallback = FSM11STD::forward<TType>(callback);
    }

protected:
    inline
    void invokeTransitionConflictAction(
            transition_type* transition, transition_type* ignoredTransition)
    {
        if (m_transitionConflictCallback)
            m_transitionConflictCallback(transition, ignoredTransition);
    }

    inline
    bool hasTransitionConflictAction() const
    {
        return m_transitionConflictCallback != nullptr;
    }

private:
    FSM11STD::function<void(transition_type* transition,
                            transition_type* ignoredTransition)>
        m_transitionConflictCallback;
};

template <typename TDerived>
class WithTransitionConflictException
{
public:
    using transition_type = Transition<TDerived>;
    using transition_conflict_error_type
        = TransitionConflictError<transition_type>;

    template <typename TType>
    void setTransitionConflictCallback(TType&&)
    {
        static_assert(!FSM11STD::is_same<TType, TType>::value,
                      "Transition conflict callbacks are disabled");
    }

protected:
    inline
    void invokeTransitionConflictAction(transition_type* first,
                                          transition_type* second)
    {
        throw FSM11_EXCEPTION(transition_conflict_error_type(first, second));
    }

    inline
    bool hasTransitionConflictAction() const
    {
        return true;
    }
};

template <typename TOptions>
struct get_transition_conflict_action
{
    using type = typename FSM11STD::conditional<
                     TOptions::transition_conflict_policy == Ignore,
                     WithoutTransitionConflictAction<StateMachineImpl<TOptions>>,
                     typename FSM11STD::conditional<
                         TOptions::transition_conflict_policy == InvokeCallback,
                         WithTransitionConflictCallback<StateMachineImpl<TOptions>>,
                         WithTransitionConflictException<StateMachineImpl<TOptions>>>::type
                 >::type;
    using type1 = WithTransitionConflictException<StateMachineImpl<TOptions>>;
};

} // namespace fsm11_detail
} // namespace fsm11

#endif // FSM11_DETAIL_CALLBACKS_HPP
