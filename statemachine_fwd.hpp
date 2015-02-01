#ifndef FSM11_STATEMACHINE_FWD_HPP
#define FSM11_STATEMACHINE_FWD_HPP

namespace fsm11
{
namespace detail
{

template <typename TOptions>
class State;

template <typename TOptions>
class StateMachine;

template <typename TOptions>
class Transition;



template <typename TType>
struct get_options;

template <typename TOptions>
struct get_options<StateMachine<TOptions>>
{
    typedef TOptions type;
};



template <typename TOptions>
State<TOptions>* findLeastCommonProperAncestor(
        State<TOptions>* state1, State<TOptions>* state2);

} // namespace detail
} // namespace fsm11

#endif // FSM11_STATEMACHINE_FWD_HPP
