#ifndef FSM11_DETAIL_CALLBACKS_HPP
#define FSM11_DETAIL_CALLBACKS_HPP

#include "statemachine_fwd.hpp"

#include <functional>
#include <utility>
#include <type_traits>

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
        static_assert(!std::is_same<TType, TType>::value,
                      "Event callbacks are disabled");
    }

    template <typename TType>
    void setEventDiscardedCallback(TType&&)
    {
        static_assert(!std::is_same<TType, TType>::value,
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
        m_eventDispatchCallback = std::forward<TType>(callback);
    }

    template <typename TType>
    void setEventDiscardedCallback(TType&& callback)
    {
        m_eventDiscardedCallback = std::forward<TType>(callback);
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
    std::function<void(event_type)> m_eventDispatchCallback;
    std::function<void(event_type)> m_eventDiscardedCallback;
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
        static_assert(!std::is_same<TType, TType>::value,
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
        m_configurationChangeCallback = std::forward<TType>(callback);
    }

protected:
    inline
    void invokeConfigurationChangeCallback()
    {
        if (m_configurationChangeCallback)
            m_configurationChangeCallback();
    }

private:
    std::function<void()> m_configurationChangeCallback;
};

template <typename TOptions>
struct get_configuration_change_callbacks
{
    using type = typename std::conditional<
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
    using options = typename get_options<TDerived>::type;
    using state_type = State<options>;

    template <typename TType>
    void setStateEntryCallback(TType&&)
    {
        static_assert(!std::is_same<TType, TType>::value,
                      "State callbacks are disabled");
    }

    template <typename TType>
    void setStateExitCallback(TType&&)
    {
        static_assert(!std::is_same<TType, TType>::value,
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
    using options = typename get_options<TDerived>::type;
    using state_type = State<options>;

    template <typename TType>
    void setStateEntryCallback(TType&& callback)
    {
        m_stateEntryCallback = std::forward<TType>(callback);
    }

    template <typename TType>
    void setStateExitCallback(TType&& callback)
    {
        m_stateExitCallback = std::forward<TType>(callback);
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
    std::function<void(state_type*)> m_stateEntryCallback;
    std::function<void(state_type*)> m_stateExitCallback;
};

template <bool TEnabled, typename TOptions>
struct get_state_callbacks_helper
{
    typedef WithoutStateCallbacks<StateMachineImpl<TOptions>> type;
};

template <typename TOptions>
struct get_state_callbacks_helper<true, TOptions>
{
    typedef WithStateCallbacks<StateMachineImpl<TOptions>> type;
};

template <typename TOptions>
struct get_state_callbacks : public get_state_callbacks_helper<TOptions::state_callbacks_enable, TOptions>
{
};

} // namespace fsm11_detail
} // namespace fsm11

#endif // FSM11_DETAIL_CALLBACKS_HPP
