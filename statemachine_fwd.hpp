#ifndef FSM11_STATEMACHINE_FWD_HPP
#define FSM11_STATEMACHINE_FWD_HPP

namespace fsm11
{

template <typename TOptions>
class State;

template <typename TOptions>
class Transition;

namespace fsm11_detail
{

template <typename TDerived>
class EventDispatcherBase;

template <typename TOptions>
class StateMachine;


template <typename TType>
struct get_options;

template <typename TOptions>
struct get_options<StateMachine<TOptions>>
{
    typedef TOptions type;
};

} // namespace detail

template <typename TOptions>
State<TOptions>* findLeastCommonProperAncestor(
        State<TOptions>* state1, State<TOptions>* state2);

} // namespace fsm11

#endif // FSM11_STATEMACHINE_FWD_HPP
