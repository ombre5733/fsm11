#ifndef STATEMACHINE_STATEMACHINE_FWD_HPP
#define STATEMACHINE_STATEMACHINE_FWD_HPP

namespace statemachine
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

} // namespace detail
} // namespace statemachine

#endif // STATEMACHINE_STATEMACHINE_FWD_HPP
