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

#ifndef FSM11_TRANSITION_HPP
#define FSM11_TRANSITION_HPP

#include "statemachine_fwd.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/functional.hpp>
#include <weos/type_traits.hpp>
#include <weos/utility.hpp>
#else
#include <functional>
#include <type_traits>
#include <utility>
#endif // FSM11_USE_WEOS

namespace fsm11
{

//! A type-tag for creating targetless transitions.
struct noTarget_t {};
struct internal_t {};
struct external_t {};

namespace fsm11_detail
{

// ----=====================================================================----
//     Intermediate types for transitions with events
// ----=====================================================================----

template <typename TState, typename TEvent, typename TGuard, typename TAction>
class TypeSourceEventGuardActionTarget
{
    static_assert(FSM11STD::is_reference<TEvent>::value, "TEvent must be a reference");
    static_assert(FSM11STD::is_reference<TGuard>::value, "TGuard must be a reference");
    static_assert(FSM11STD::is_reference<TAction>::value, "TAction must be a reference");

public:
    constexpr
    TypeSourceEventGuardActionTarget(TState* source, TState* target,
                                     TEvent event, TGuard guard, TAction action,
                                     bool isExternal = true) noexcept
        : m_source(source),
          m_target(target),
          m_event(FSM11STD::forward<TEvent>(event)),
          m_guard(FSM11STD::forward<TGuard>(guard)),
          m_action(FSM11STD::forward<TAction>(action)),
          m_isExternal(isExternal)
    {
    }

    constexpr
    TypeSourceEventGuardActionTarget(TypeSourceEventGuardActionTarget&& other) noexcept
        : m_source(other.m_source),
          m_target(other.m_target),
          m_event(FSM11STD::forward<TEvent>(other.m_event)),
          m_guard(FSM11STD::forward<TGuard>(other.m_guard)),
          m_action(FSM11STD::forward<TAction>(other.m_action)),
          m_isExternal(other.m_isExternal)
    {
    }

    TypeSourceEventGuardActionTarget(const TypeSourceEventGuardActionTarget&) = delete;
    TypeSourceEventGuardActionTarget& operator=(const TypeSourceEventGuardActionTarget&) = delete;

private:
    TState* m_source;
    TState* m_target;
    TEvent m_event;
    TGuard m_guard;
    TAction m_action;
    bool m_isExternal;

    template <typename TStateMachine>
    friend class fsm11::Transition;
};

template <typename TState, typename TEvent, typename TGuard, typename TAction>
class TypeSourceEventGuardAction
{
    static_assert(FSM11STD::is_reference<TEvent>::value, "TEvent must be a reference");
    static_assert(FSM11STD::is_reference<TGuard>::value, "TGuard must be a reference");
    static_assert(FSM11STD::is_reference<TAction>::value, "TAction must be a reference");

public:
    constexpr
    TypeSourceEventGuardAction(TState* source, TEvent event, TGuard guard,
                               TAction action, bool isExternal) noexcept
        : m_source(source),
          m_event(FSM11STD::forward<TEvent>(event)),
          m_guard(FSM11STD::forward<TGuard>(guard)),
          m_action(FSM11STD::forward<TAction>(action)),
          m_isExternal(isExternal)
    {
    }

    constexpr
    TypeSourceEventGuardAction(TypeSourceEventGuardAction&& other) noexcept
        : m_source(other.m_source),
          m_event(FSM11STD::forward<TEvent>(other.m_event)),
          m_guard(FSM11STD::forward<TGuard>(other.m_guard)),
          m_action(FSM11STD::forward<TAction>(other.m_action)),
          m_isExternal(other.m_isExternal)
    {
    }

    TypeSourceEventGuardAction(const TypeSourceEventGuardAction&) = delete;
    TypeSourceEventGuardAction& operator=(const TypeSourceEventGuardAction&) = delete;

    constexpr
    TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction> operator>(
            TState& target) const noexcept
    {
        return TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>(
                    m_source, &target,
                    FSM11STD::forward<TEvent>(m_event),
                    FSM11STD::forward<TGuard>(m_guard),
                    FSM11STD::forward<TAction>(m_action),
                    m_isExternal);
    }

    constexpr
    TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction> operator>(
            noTarget_t) const noexcept
    {
        return TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>(
                    m_source, nullptr,
                    FSM11STD::forward<TEvent>(m_event),
                    FSM11STD::forward<TGuard>(m_guard),
                    FSM11STD::forward<TAction>(m_action),
                    m_isExternal);
    }

private:
    TState* m_source;
    TEvent m_event;
    TGuard m_guard;
    TAction m_action;
    bool m_isExternal;
};

template <typename TState, typename TEvent, typename TGuard, typename TAction>
class SourceEventGuardAction
{
    static_assert(FSM11STD::is_reference<TEvent>::value, "TEvent must be a reference");
    static_assert(FSM11STD::is_reference<TGuard>::value, "TGuard must be a reference");
    static_assert(FSM11STD::is_reference<TAction>::value, "TAction must be a reference");

public:
    constexpr
    SourceEventGuardAction(TState* source, TEvent event, TGuard guard,
                           TAction action) noexcept
        : m_source(source),
          m_event(FSM11STD::forward<TEvent>(event)),
          m_guard(FSM11STD::forward<TGuard>(guard)),
          m_action(FSM11STD::forward<TAction>(action))
    {
    }

    constexpr
    SourceEventGuardAction(SourceEventGuardAction&& other) noexcept
        : m_source(other.m_source),
          m_event(FSM11STD::forward<TEvent>(other.m_event)),
          m_guard(FSM11STD::forward<TGuard>(other.m_guard)),
          m_action(FSM11STD::forward<TAction>(other.m_action))
    {
    }

    SourceEventGuardAction(const SourceEventGuardAction&) = delete;
    SourceEventGuardAction& operator=(const SourceEventGuardAction&) = delete;

    constexpr
    TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction> operator>(
            TState& target) const noexcept
    {
        return TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>(
                    m_source, &target,
                    FSM11STD::forward<TEvent>(m_event),
                    FSM11STD::forward<TGuard>(m_guard),
                    FSM11STD::forward<TAction>(m_action));
    }

    constexpr
    TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction> operator>(
            noTarget_t) const noexcept
    {
        return TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>(
                    m_source, nullptr,
                    FSM11STD::forward<TEvent>(m_event),
                    FSM11STD::forward<TGuard>(m_guard),
                    FSM11STD::forward<TAction>(m_action));
    }

    TState* m_source;
    TEvent m_event;
    TGuard m_guard;
    TAction m_action;
};

template <typename TEvent, typename TGuard, typename TAction>
struct EventGuardAction
{
    static_assert(FSM11STD::is_reference<TEvent>::value, "TEvent must be a reference");
    static_assert(FSM11STD::is_reference<TGuard>::value, "TGuard must be a reference");
    static_assert(FSM11STD::is_reference<TAction>::value, "TAction must be a reference");

    constexpr
    EventGuardAction(TEvent event, TGuard guard, TAction action) noexcept
        : m_event(FSM11STD::forward<TEvent>(event)),
          m_guard(FSM11STD::forward<TGuard>(guard)),
          m_action(FSM11STD::forward<TAction>(action))
    {
    }

    constexpr
    EventGuardAction(EventGuardAction&& other) noexcept
        : m_event(FSM11STD::forward<TEvent>(other.m_event)),
          m_guard(FSM11STD::forward<TGuard>(other.m_guard)),
          m_action(FSM11STD::forward<TAction>(other.m_action))
    {
    }

    EventGuardAction(const EventGuardAction&) = delete;
    EventGuardAction& operator=(const EventGuardAction&) = delete;

    TEvent m_event;
    TGuard m_guard;
    TAction m_action;
};

template <typename TEvent, typename TGuard>
struct EventGuard
{
    static_assert(FSM11STD::is_reference<TEvent>::value, "TEvent must be a reference");
    static_assert(FSM11STD::is_reference<TGuard>::value, "TGuard must be a reference");

    constexpr
    EventGuard(TEvent event, TGuard guard) noexcept
        : m_event(FSM11STD::forward<TEvent>(event)),
          m_guard(FSM11STD::forward<TGuard>(guard))
    {
    }

    constexpr
    EventGuard(EventGuard&& other) noexcept
        : m_event(FSM11STD::forward<TEvent>(other.m_event)),
          m_guard(FSM11STD::forward<TGuard>(other.m_guard))
    {
    }

    EventGuard(const EventGuard&) = delete;
    EventGuard& operator=(const EventGuard&) = delete;

    template <typename TAction>
    constexpr
    EventGuardAction<TEvent, TGuard, TAction&&> operator/(TAction&& action) const noexcept
    {
        return EventGuardAction<TEvent, TGuard, TAction&&>(
                   FSM11STD::forward<TEvent>(m_event),
                   FSM11STD::forward<TGuard>(m_guard),
                   FSM11STD::forward<TAction>(action));
    }

    TEvent m_event;
    TGuard m_guard;
};

template <typename TEvent>
struct Event
{
    static_assert(FSM11STD::is_reference<TEvent>::value, "TEvent must be a reference");

    constexpr
    Event(TEvent event) noexcept
        : m_event(FSM11STD::forward<TEvent>(event))
    {
    }

    constexpr
    Event(Event&& other) noexcept
        : m_event(FSM11STD::forward<TEvent>(other.m_event))
    {
    }

    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    template <typename TGuard>
    constexpr
    EventGuard<TEvent, TGuard&&> operator[](TGuard&& guard) const noexcept
    {
        return EventGuard<TEvent, TGuard&&>(FSM11STD::forward<TEvent>(m_event),
                                            FSM11STD::forward<TGuard>(guard));
    }

    template <typename TGuard>
    constexpr
    EventGuard<TEvent, TGuard&&> operator()(TGuard&& guard) const noexcept
    {
        return EventGuard<TEvent, TGuard&&>(FSM11STD::forward<TEvent>(m_event),
                                            FSM11STD::forward<TGuard>(guard));
    }

    template <typename TAction>
    constexpr
    EventGuardAction<TEvent, FSM11STD::nullptr_t&&, TAction&&> operator/(
            TAction&& action) const noexcept
    {
        return EventGuardAction<TEvent, FSM11STD::nullptr_t&&, TAction&&>(
                   FSM11STD::forward<TEvent>(m_event),
                   nullptr,
                   FSM11STD::forward<TAction>(action));
    }

    TEvent m_event;
};

template <typename TSm, typename TEvent>
constexpr
auto operator+(State<TSm>& source, Event<TEvent>&& rhs) noexcept
    -> SourceEventGuardAction<State<TSm>, TEvent, FSM11STD::nullptr_t&&, FSM11STD::nullptr_t&&>
{
    return SourceEventGuardAction<State<TSm>, TEvent, FSM11STD::nullptr_t&&, FSM11STD::nullptr_t&&>(
                &source,
                FSM11STD::forward<TEvent>(rhs.m_event), nullptr, nullptr);
}

template <typename TSm, typename TEvent, typename TGuard>
constexpr
auto operator+(State<TSm>& source, EventGuard<TEvent, TGuard>&& rhs) noexcept
    -> SourceEventGuardAction<State<TSm>, TEvent, TGuard, FSM11STD::nullptr_t&&>
{
    return SourceEventGuardAction<State<TSm>, TEvent, TGuard, FSM11STD::nullptr_t&&>(
                &source,
                FSM11STD::forward<TEvent>(rhs.m_event),
                FSM11STD::forward<TGuard>(rhs.m_guard), nullptr);
}

template <typename TSm, typename TEvent, typename TGuard, typename TAction>
constexpr
auto operator+(State<TSm>& source, EventGuardAction<TEvent, TGuard, TAction>&& rhs) noexcept
    -> SourceEventGuardAction<State<TSm>, TEvent, TGuard, TAction>
{
    return SourceEventGuardAction<State<TSm>, TEvent, TGuard, TAction>(
                &source,
                FSM11STD::forward<TEvent>(rhs.m_event),
                FSM11STD::forward<TGuard>(rhs.m_guard),
                FSM11STD::forward<TAction>(rhs.m_action));
}

template <typename TSm, typename TEvent, typename TGuard, typename TAction>
constexpr
auto operator>(internal_t, SourceEventGuardAction<State<TSm>, TEvent, TGuard, TAction>&& rhs) noexcept
    -> TypeSourceEventGuardAction<State<TSm>, TEvent, TGuard, TAction>
{
    return TypeSourceEventGuardAction<State<TSm>, TEvent, TGuard, TAction>(
                rhs.m_source,
                FSM11STD::forward<TEvent>(rhs.m_event),
                FSM11STD::forward<TGuard>(rhs.m_guard),
                FSM11STD::forward<TAction>(rhs.m_action),
                false);
}

template <typename TSm, typename TEvent, typename TGuard, typename TAction>
constexpr
auto operator>(external_t, SourceEventGuardAction<State<TSm>, TEvent, TGuard, TAction>&& rhs) noexcept
    -> TypeSourceEventGuardAction<State<TSm>, TEvent, TGuard, TAction>
{
    return TypeSourceEventGuardAction<State<TSm>, TEvent, TGuard, TAction>(
                rhs.m_source,
                FSM11STD::forward<TEvent>(rhs.m_event),
                FSM11STD::forward<TGuard>(rhs.m_guard),
                FSM11STD::forward<TAction>(rhs.m_action),
                true);
}

// ----=====================================================================----
//     Intermediate types for transitions without events
// ----=====================================================================----

template <typename TState, typename TGuard, typename TAction>
class TypeSourceNoEventGuardActionTarget
{
    static_assert(FSM11STD::is_reference<TGuard>::value, "TGuard must be a reference");
    static_assert(FSM11STD::is_reference<TAction>::value, "TAction must be a reference");

public:
    constexpr
    TypeSourceNoEventGuardActionTarget(TState* source, TState* target,
                                       TGuard guard, TAction action,
                                       bool isExternal = true) noexcept
        : m_source(source),
          m_target(target),
          m_guard(FSM11STD::forward<TGuard>(guard)),
          m_action(FSM11STD::forward<TAction>(action)),
          m_isExternal(isExternal)
    {
    }

    constexpr
    TypeSourceNoEventGuardActionTarget(TypeSourceNoEventGuardActionTarget&& other) noexcept
        : m_source(other.m_source),
          m_target(other.m_target),
          m_guard(FSM11STD::forward<TGuard>(other.m_guard)),
          m_action(FSM11STD::forward<TAction>(other.m_action)),
          m_isExternal(other.m_isExternal)
    {
    }

    TypeSourceNoEventGuardActionTarget(const TypeSourceNoEventGuardActionTarget&) = delete;
    TypeSourceNoEventGuardActionTarget& operator=(const TypeSourceNoEventGuardActionTarget&) = delete;

private:
    TState* m_source;
    TState* m_target;
    TGuard m_guard;
    TAction m_action;
    bool m_isExternal;

    template <typename TStateMachine>
    friend class fsm11::Transition;
};


template <typename TState, typename TGuard, typename TAction>
class TypeSourceNoEventGuardAction
{
    static_assert(FSM11STD::is_reference<TGuard>::value, "TGuard must be a reference");
    static_assert(FSM11STD::is_reference<TAction>::value, "TAction must be a reference");

public:
    constexpr
    TypeSourceNoEventGuardAction(TState* source, TGuard guard,
                                 TAction action, bool isExternal) noexcept
        : m_source(source),
          m_guard(FSM11STD::forward<TGuard>(guard)),
          m_action(FSM11STD::forward<TAction>(action)),
          m_isExternal(isExternal)
    {
    }

    constexpr
    TypeSourceNoEventGuardAction(TypeSourceNoEventGuardAction&& other) noexcept
        : m_source(other.m_source),
          m_guard(FSM11STD::forward<TGuard>(other.m_guard)),
          m_action(FSM11STD::forward<TAction>(other.m_action)),
          m_isExternal(other.m_isExternal)
    {
    }

    TypeSourceNoEventGuardAction(const TypeSourceNoEventGuardAction&) = delete;
    TypeSourceNoEventGuardAction& operator=(const TypeSourceNoEventGuardAction&) = delete;

    constexpr
    TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction> operator>(
            TState& target) const noexcept
    {
        return TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction>(
                    m_source, &target,
                    FSM11STD::forward<TGuard>(m_guard),
                    FSM11STD::forward<TAction>(m_action),
                    m_isExternal);
    }

    constexpr
    TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction> operator>(
            noTarget_t) const noexcept
    {
        return TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction>(
                    m_source, nullptr,
                    FSM11STD::forward<TGuard>(m_guard),
                    FSM11STD::forward<TAction>(m_action),
                    m_isExternal);
    }

private:
    TState* m_source;
    TGuard m_guard;
    TAction m_action;
    bool m_isExternal;
};

template <typename TState, typename TGuard, typename TAction>
class SourceNoEventGuardAction
{
    static_assert(FSM11STD::is_reference<TGuard>::value, "TGuard must be a reference");
    static_assert(FSM11STD::is_reference<TAction>::value, "TAction must be a reference");

public:
    constexpr
    SourceNoEventGuardAction(TState* source, TGuard guard, TAction action) noexcept
        : m_source(source),
          m_guard(FSM11STD::forward<TGuard>(guard)),
          m_action(FSM11STD::forward<TAction>(action))
    {
    }

    constexpr
    SourceNoEventGuardAction(SourceNoEventGuardAction&& other) noexcept
        : m_source(other.m_source),
          m_guard(FSM11STD::forward<TGuard>(other.m_guard)),
          m_action(FSM11STD::forward<TAction>(other.m_action))
    {
    }

    SourceNoEventGuardAction(const SourceNoEventGuardAction&) = delete;
    SourceNoEventGuardAction& operator=(const SourceNoEventGuardAction&) = delete;

    constexpr
    TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction> operator>(
            TState& target) const noexcept
    {
        return TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction>(
                    m_source, &target,
                    FSM11STD::forward<TGuard>(m_guard),
                    FSM11STD::forward<TAction>(m_action));
    }

    constexpr
    TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction> operator>(
            noTarget_t) const noexcept
    {
        return TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction>(
                    m_source, nullptr,
                    FSM11STD::forward<TGuard>(m_guard),
                    FSM11STD::forward<TAction>(m_action));
    }

    TState* m_source;
    TGuard m_guard;
    TAction m_action;
};

template <typename TGuard, typename TAction>
struct NoEventGuardAction
{
    static_assert(FSM11STD::is_reference<TGuard>::value, "TGuard must be a reference");
    static_assert(FSM11STD::is_reference<TAction>::value, "TAction must be a reference");

    constexpr
    NoEventGuardAction(TGuard guard, TAction action) noexcept
        : m_guard(FSM11STD::forward<TGuard>(guard)),
          m_action(FSM11STD::forward<TAction>(action))
    {
    }

    constexpr
    NoEventGuardAction(NoEventGuardAction&& other) noexcept
        : m_guard(FSM11STD::forward<TGuard>(other.m_guard)),
          m_action(FSM11STD::forward<TAction>(other.m_action))
    {
    }

    NoEventGuardAction(const NoEventGuardAction&) = delete;
    NoEventGuardAction& operator=(const NoEventGuardAction&) = delete;

    TGuard m_guard;
    TAction m_action;
};

template <typename TGuard>
struct NoEventGuard
{
    static_assert(FSM11STD::is_reference<TGuard>::value, "TGuard must be a reference");

    constexpr
    NoEventGuard(TGuard guard) noexcept
        : m_guard(FSM11STD::forward<TGuard>(guard))
    {
    }

    constexpr
    NoEventGuard(NoEventGuard&& other) noexcept
        : m_guard(FSM11STD::forward<TGuard>(other.m_guard))
    {
    }

    NoEventGuard(const NoEventGuard&) = delete;
    NoEventGuard& operator=(const NoEventGuard&) = delete;

    template <typename TAction>
    constexpr
    NoEventGuardAction<TGuard, TAction&&> operator/(TAction&& action) const noexcept
    {
        return NoEventGuardAction<TGuard, TAction&&>(
                   FSM11STD::forward<TGuard>(m_guard),
                   FSM11STD::forward<TAction>(action));
    }

    TGuard m_guard;
};

class NoEvent
{
public:
    template <typename TGuard>
    constexpr
    NoEventGuard<TGuard&&> operator[](TGuard&& guard) const noexcept
    {
        return NoEventGuard<TGuard&&>(FSM11STD::forward<TGuard>(guard));
    }

    template <typename TGuard>
    constexpr
    NoEventGuard<TGuard&&> operator()(TGuard&& guard) const noexcept
    {
        return NoEventGuard<TGuard&&>(FSM11STD::forward<TGuard>(guard));
    }

    template <typename TAction>
    constexpr
    NoEventGuardAction<FSM11STD::nullptr_t&&, TAction&&> operator/(
            TAction&& action) const noexcept
    {
        return NoEventGuardAction<FSM11STD::nullptr_t&&, TAction&&>(
                   nullptr,
                   FSM11STD::forward<TAction>(action));
    }
};

template <typename TSm>
constexpr
auto operator+(State<TSm>& source, NoEvent) noexcept
    -> SourceNoEventGuardAction<State<TSm>, FSM11STD::nullptr_t&&, FSM11STD::nullptr_t&&>
{
    return SourceNoEventGuardAction<State<TSm>, FSM11STD::nullptr_t&&, FSM11STD::nullptr_t&&>(
                &source, nullptr, nullptr);
}

template <typename TSm, typename TGuard>
constexpr
auto operator+(State<TSm>& source, NoEventGuard<TGuard>&& rhs) noexcept
    -> SourceNoEventGuardAction<State<TSm>, TGuard, FSM11STD::nullptr_t&&>
{
    return SourceNoEventGuardAction<State<TSm>, TGuard, FSM11STD::nullptr_t&&>(
                &source,
                FSM11STD::forward<TGuard>(rhs.m_guard), nullptr);
}

template <typename TSm, typename TGuard, typename TAction>
constexpr
auto operator+(State<TSm>& source, NoEventGuardAction<TGuard, TAction>&& rhs) noexcept
    -> SourceNoEventGuardAction<State<TSm>, TGuard, TAction>
{
    return SourceNoEventGuardAction<State<TSm>, TGuard, TAction>(
                &source,
                FSM11STD::forward<TGuard>(rhs.m_guard),
                FSM11STD::forward<TAction>(rhs.m_action));
}

template <typename TSm, typename TGuard, typename TAction>
constexpr
auto operator>(internal_t, SourceNoEventGuardAction<State<TSm>, TGuard, TAction>&& rhs) noexcept
    -> TypeSourceNoEventGuardAction<State<TSm>, TGuard, TAction>
{
    return TypeSourceNoEventGuardAction<State<TSm>, TGuard, TAction>(
                rhs.m_source,
                FSM11STD::forward<TGuard>(rhs.m_guard),
                FSM11STD::forward<TAction>(rhs.m_action),
                false);
}

template <typename TSm, typename TGuard, typename TAction>
constexpr
auto operator>(external_t, SourceNoEventGuardAction<State<TSm>, TGuard, TAction>&& rhs) noexcept
    -> TypeSourceNoEventGuardAction<State<TSm>, TGuard, TAction>
{
    return TypeSourceNoEventGuardAction<State<TSm>, TGuard, TAction>(
                rhs.m_source,
                FSM11STD::forward<TGuard>(rhs.m_guard),
                FSM11STD::forward<TAction>(rhs.m_action),
                true);
}

} // namespace fsm11_detail

// ----=====================================================================----
//     Transition
// ----=====================================================================----

//! \brief A transition.
template <typename TStateMachine>
class Transition
{
    using options = typename fsm11_detail::get_options<TStateMachine>::type;

public:
    using state_type = State<TStateMachine>;
    using event_type = typename options::event_type;
    using action_type = FSM11STD::function<void(event_type)>;
    using guard_type = FSM11STD::function<bool(event_type)>;

    //! \brief Creates a transition.
    //!
    //! Creates a transition from the specification \p rhs.
    template <typename TState, typename TEvent, typename TGuard,
              typename TAction>
    explicit Transition(fsm11_detail::TypeSourceEventGuardActionTarget<
                            TState, TEvent, TGuard, TAction>&& rhs)
        : m_source(rhs.m_source),
          m_target(rhs.m_target),
          m_nextInSourceState{nullptr},
          m_nextInEnabledSet{nullptr},
          m_guard{FSM11STD::forward<TGuard>(rhs.m_guard)},
          m_action{FSM11STD::forward<TAction>(rhs.m_action)},
          m_event{FSM11STD::forward<TEvent>(rhs.m_event)},
          m_eventless(false),
          m_isExternal(rhs.m_isExternal)
    {
    }

    //! \brief Creates a transition.
    //!
    //! Creates an eventless transition from the specification \p rhs.
    template <typename TState, typename TGuard, typename TAction>
    explicit Transition(fsm11_detail::TypeSourceNoEventGuardActionTarget<
                            TState, TGuard, TAction>&& rhs)
        : m_source(rhs.m_source),
          m_target(rhs.m_target),
          m_nextInSourceState{nullptr},
          m_nextInEnabledSet{nullptr},
          m_guard{FSM11STD::forward<TGuard>(rhs.m_guard)},
          m_action{FSM11STD::forward<TAction>(rhs.m_action)},
          m_event(),
          m_eventless(true),
          m_isExternal(rhs.m_isExternal)
    {
    }

    Transition(const Transition&) = delete;
    Transition& operator=(const Transition&) = delete;

    //! \brief Returns the action.
    //!
    //! Returns the transition's action.
    const action_type& action() const noexcept
    {
        return m_action;
    }

    //! \brief Returns the guard.
    //!
    //! Returns the transition's guard function.
    const guard_type& guard() const noexcept
    {
        return m_guard;
    }

    //! \brief Returns the event.
    //!
    //! Returns the transition's event.
    const event_type& event() const noexcept
    {
        return m_event;
    }

    //! \brief Checks if the transition is eventless.
    //!
    //! Returns \p true, if this transition is eventless.
    bool eventless() const noexcept
    {
        return m_eventless;
    }

    //! \brief Checks if the transition is external.
    //!
    //! Returns \p true, if this transition is an external one.
    bool isExternal() const
    {
        return m_isExternal;
    }

    //! \brief Checks if the transition is internal.
    //!
    //! Returns \p true, if this transition is an internal one.
    bool isInternal() const
    {
        return !m_isExternal;
    }

    //! \brief The source state.
    //!
    //! Returns the source state.
    state_type* source() const noexcept
    {
        return m_source;
    }

    //! \brief The target state.
    //!
    //! Returns the target state. If this is a targetless transition, a
    //! null-pointer is returned.
    state_type* target() const noexcept
    {
        return m_target;
    }

private:
    state_type* m_source;
    state_type* m_target;

    //! The next transition in the same source state.
    Transition* m_nextInSourceState;

    //! The next transition in the set of enabled transitions.
    Transition* m_nextInEnabledSet;

    guard_type m_guard;
    action_type m_action;

    event_type m_event;
    bool m_eventless;
    bool m_isExternal;


    friend state_type;
    friend TStateMachine;

    template <typename TDerived>
    friend class fsm11_detail::EventDispatcherBase;
};

//! \brief Names an event in a transition specification.
template <typename TEvent>
inline
fsm11_detail::Event<TEvent&&> event(TEvent&& ev) noexcept
{
    return fsm11_detail::Event<TEvent&&>(FSM11STD::forward<TEvent>(ev));
}

//! A tag to create eventless transitions.
constexpr fsm11_detail::NoEvent noEvent = fsm11_detail::NoEvent();

//! A tag to create targetless transitions.
constexpr noTarget_t noTarget = noTarget_t();

//! A tag to create an internal transition.
constexpr internal_t internal = internal_t();

//! A tag to create an external transition.
constexpr external_t external = external_t();

} // namespace fsm11

#endif // FSM11_TRANSITION_HPP
