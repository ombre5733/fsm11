#ifndef FSM11_STATEMACHINE_FWD_HPP
#define FSM11_STATEMACHINE_FWD_HPP

// If the compile switch FSM11_USER_CONFIG is set, it points to the user's
// configuration file. If the switch is not set, we assume that the
// user configuration is somewhere in the path.
#if defined(FSM11_USER_CONFIG)
    #include FSM11_USER_CONFIG
#else
    #include "fsm11_user_config.hpp"
#endif // FSM11_USER_CONFIG



#ifdef FSM11_USE_WEOS
    #define FSM11STD   weos
#else
    #define FSM11STD   std
#endif // FSM11_USE_WEOS


namespace fsm11
{

template <typename TStateMachine>
class State;

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

} // namespace detail

template <typename TStateMachine>
State<TStateMachine>* findLeastCommonProperAncestor(
        State<TStateMachine>* state1, State<TStateMachine>* state2) noexcept;

} // namespace fsm11

#endif // FSM11_STATEMACHINE_FWD_HPP
