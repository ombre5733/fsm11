#ifndef FSM11_STATEMACHINE_FWD_HPP
#define FSM11_STATEMACHINE_FWD_HPP

#define FSM11STD   std

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
        State<TStateMachine>* state1, State<TStateMachine>* state2);

} // namespace fsm11

#endif // FSM11_STATEMACHINE_FWD_HPP
