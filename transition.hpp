#ifndef FSM11_TRANSITION_HPP
#define FSM11_TRANSITION_HPP

#include "statemachine_fwd.hpp"

#include <functional>
#include <type_traits>

namespace fsm11
{
namespace detail
{

//! A type-tag for creating eventless transitions.
struct noEvent_t {};
//! A type-tag for creating targetless transitions.
struct noTarget_t {};

//! A tag for creating eventless transitions.
constexpr noEvent_t noEvent = noEvent_t();
//! A tag for creating targetless transitions.
constexpr noTarget_t noTarget = noTarget_t();



template <typename TEvent, typename TGuard, typename TAction>
class NoEventGuardAction
{
};

template <typename TGuard>
class NoEventGuard
{
};

template <typename TState, typename TEvent, typename TGuard, typename TAction>
class SourceEventGuardActionTarget
{
    static_assert(std::is_reference<TEvent>::value, "TEvent must be a reference");
    static_assert(std::is_reference<TGuard>::value, "TGuard must be a reference");
    static_assert(std::is_reference<TAction>::value, "TAction must be a reference");

public:
    SourceEventGuardActionTarget(TState* source, TState* target,
                                 TEvent event, TGuard guard, TAction action)
        : m_source(source),
          m_target(target),
          m_event(std::forward<TEvent>(event)),
          m_guard(std::forward<TGuard>(guard)),
          m_action(std::forward<TAction>(action))
    {
    }

private:
    TState* m_source;
    TState* m_target;
    TEvent m_event;
    TGuard m_guard;
    TAction m_action;

    template <typename TOptions>
    friend class Transition;
};

template <typename TState, typename TEvent, typename TGuard, typename TAction>
class SourceEventGuardAction
{
    static_assert(std::is_reference<TEvent>::value, "TEvent must be a reference");
    static_assert(std::is_reference<TGuard>::value, "TGuard must be a reference");
    static_assert(std::is_reference<TAction>::value, "TAction must be a reference");

public:
    SourceEventGuardAction(TState* source, TEvent event, TGuard guard,
                           TAction action)
        : m_source(source),
          m_event(std::forward<TEvent>(event)),
          m_guard(std::forward<TGuard>(guard)),
          m_action(std::forward<TAction>(action))
    {
    }

    SourceEventGuardActionTarget<TState, TEvent, TGuard, TAction> operator==(
            TState& target) const
    {
        return SourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>(
                    m_source, &target,
                    std::forward<TEvent>(m_event),
                    std::forward<TGuard>(m_guard),
                    std::forward<TAction>(m_action));
    }

    SourceEventGuardActionTarget<TState, TEvent, TGuard, TAction> operator==(
            noTarget_t) const
    {
        return SourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>(
                    m_source, 0,
                    std::forward<TEvent>(m_event),
                    std::forward<TGuard>(m_guard),
                    std::forward<TAction>(m_action));
    }

private:
    TState* m_source;
    TEvent m_event;
    TGuard m_guard;
    TAction m_action;
};

template <typename TEvent, typename TGuard, typename TAction>
struct EventGuardAction
{
    static_assert(std::is_reference<TEvent>::value, "TEvent must be a reference");
    static_assert(std::is_reference<TGuard>::value, "TGuard must be a reference");
    static_assert(std::is_reference<TAction>::value, "TAction must be a reference");

    EventGuardAction(TEvent event, TGuard guard, TAction action)
        : m_event(std::forward<TEvent>(event)),
          m_guard(std::forward<TGuard>(guard)),
          m_action(std::forward<TAction>(action))
    {
    }

    EventGuardAction(const EventGuardAction&) = default;
    EventGuardAction& operator=(const EventGuardAction&) = delete;

    TEvent m_event;
    TGuard m_guard;
    TAction m_action;
};

template <typename TEvent, typename TGuard>
struct EventGuard
{
    static_assert(std::is_reference<TEvent>::value, "TEvent must be a reference");
    static_assert(std::is_reference<TGuard>::value, "TGuard must be a reference");

    EventGuard(TEvent event, TGuard guard)
        : m_event(std::forward<TEvent>(event)),
          m_guard(std::forward<TGuard>(guard))
    {
    }

    EventGuard(const EventGuard&) = default;
    EventGuard& operator=(const EventGuard&) = delete;

    template <typename TAction>
    EventGuardAction<TEvent, TGuard, TAction&&> operator/ (TAction&& action) const
    {
        return EventGuardAction<TEvent, TGuard, TAction&&>(
                   std::forward<TEvent>(m_event),
                   std::forward<TGuard>(m_guard),
                   std::forward<TAction>(action));
    }

    TEvent m_event;
    TGuard m_guard;
};

template <typename TEvent>
struct Event
{
    static_assert(std::is_reference<TEvent>::value, "TEvent must be a reference");

    Event(TEvent event)
        : m_event(std::forward<TEvent>(event))
    {
    }

    Event(const Event&) = default;
    Event& operator=(const Event&) = delete;

    template <typename TGuard>
    EventGuard<TEvent, TGuard&&> operator[](TGuard&& guard) const
    {
        return EventGuard<TEvent, TGuard&&>(std::forward<TEvent>(m_event),
                                            std::forward<TGuard>(guard));
    }

    template <typename TGuard>
    EventGuard<TEvent, TGuard&&> operator()(TGuard&& guard) const
    {
        return EventGuard<TEvent, TGuard&&>(std::forward<TEvent>(m_event),
                                            std::forward<TGuard>(guard));
    }

    template <typename TAction>
    EventGuardAction<TEvent, std::nullptr_t&&, TAction&&> operator/(TAction&& action) const
    {
        return EventGuardAction<TEvent, std::nullptr_t&&, TAction&&>(
                   std::forward<TEvent>(m_event),
                   nullptr,
                   std::forward<TAction>(action));
    }

    TEvent m_event;
};

template <typename TOptions, typename TEvent>
auto operator+(State<TOptions>& source, Event<TEvent>&& rhs)
    -> SourceEventGuardAction<State<TOptions>, TEvent, std::nullptr_t&&, std::nullptr_t&&>
{
    return SourceEventGuardAction<State<TOptions>, TEvent, std::nullptr_t&&, std::nullptr_t&&>(
                &source,
                std::forward<TEvent>(rhs.m_event), nullptr, nullptr);
}

template <typename TOptions, typename TEvent, typename TGuard>
auto operator+(State<TOptions>& source, EventGuard<TEvent, TGuard>&& rhs)
    -> SourceEventGuardAction<State<TOptions>, TEvent, TGuard, std::nullptr_t&&>
{
    return SourceEventGuardAction<State<TOptions>, TEvent, TGuard, std::nullptr_t&&>(
                &source,
                std::forward<TEvent>(rhs.m_event),
                std::forward<TGuard>(rhs.m_guard), nullptr);
}

template <typename TOptions, typename TEvent, typename TGuard, typename TAction>
auto operator+(State<TOptions>& source, EventGuardAction<TEvent, TGuard, TAction>&& rhs)
    -> SourceEventGuardAction<State<TOptions>, TEvent, TGuard, TAction>
{
    return SourceEventGuardAction<State<TOptions>, TEvent, TGuard, TAction>(
                &source,
                std::forward<TEvent>(rhs.m_event),
                std::forward<TGuard>(rhs.m_guard),
                std::forward<TAction>(rhs.m_action));
}

//! \brief A transition.
template <typename TOptions>
class Transition
{
public:
    using state_type = State<TOptions>;
    using event_type = typename TOptions::event_type;
    using action_type = std::function<void(event_type)>;
    using guard_type = std::function<bool(event_type)>;

    template <typename TState, typename TEvent, typename TGuard,
              typename TAction>
    explicit Transition(
            SourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>&& rhs)
        : m_source(rhs.m_source),
          m_target(rhs.m_target),
          m_nextInSourceState(0),
          m_nextInEnabledSet(0),
          m_guard(std::forward<TGuard>(rhs.m_guard)),
          m_action(std::forward<TAction>(rhs.m_action)),
          m_event(std::forward<TEvent>(rhs.m_event)),
          m_eventless(false)
    {
    }

    Transition(const Transition&) = delete;
    Transition& operator=(const Transition&) = delete;

    const action_type& action() const
    {
        return m_action;
    }

    const guard_type& guard() const
    {
        return m_guard;
    }

    const event_type& event() const
    {
        return m_event;
    }

    //! \brief Checks if the transition is eventless.
    //!
    //! Returns \p true, if this transition is eventless.
    bool eventless() const
    {
        return m_eventless;
    }

    //! \brief The source state.
    //!
    //! Returns the source state.
    state_type* source() const
    {
        return m_source;
    }

    //! \brief The target state.
    //!
    //! Returns the target state. If this is a targetless transition, a
    //! null-pointer is returned.
    state_type* target() const
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


    friend class State<TOptions>;
    friend class StateMachine<TOptions>;

    template <typename TDerived>
    friend class EventDispatcherBase;
};

} // namespace detail


template <typename TEvent>
inline
detail::Event<TEvent&&> event(TEvent&& ev)
{
    return detail::Event<TEvent&&>(std::forward<TEvent>(ev));
}

} // namespace fsm11

#endif // FSM11_TRANSITION_HPP
