#ifndef FSM11_FUNCTIONSTATE_HPP
#define FSM11_FUNCTIONSTATE_HPP

#include "state.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/functional.hpp>
#include <weos/utility.hpp>
#else
#include <functional>
#include <utility>
#endif // FSM11_USE_WEOS

namespace fsm11
{

template <typename TStateMachine>
class FunctionState : public State<TStateMachine>
{
    using base_type = State<TStateMachine>;
    using options = typename fsm11_detail::get_options<TStateMachine>::type;

public:
    using event_type = typename options::event_type;
    using function_type = FSM11STD::function<void(event_type)>;
    using type = FunctionState<TStateMachine>;

    template <typename TEntry, typename TExit>
    explicit FunctionState(const char* name,
                           TEntry&& entryFn, TExit&& exitFn,
                           base_type* parent = 0)
        : base_type(name, parent),
          m_entryFunction(FSM11STD::forward<TEntry>(entryFn)),
          m_exitFunction(FSM11STD::forward<TExit>(exitFn))
    {
    }

    FunctionState(const FunctionState&) = delete;
    FunctionState& operator=(const FunctionState&) = delete;

    //! Returns the entry function.
    const function_type& entryFunction() const
    {
        return m_entryFunction;
    }

    template <typename T>
    void setEntryFunction(T&& fn)
    {
        m_entryFunction = FSM11STD::forward<T>(fn);
    }

    //! Returns the exit function.
    const function_type& exitFunction() const
    {
        return m_exitFunction;
    }

    template <typename T>
    void setExitFunction(T&& fn)
    {
        m_exitFunction = FSM11STD::forward<T>(fn);
    }

    virtual void onEntry(event_type event) override
    {
        if (m_entryFunction)
            m_entryFunction(event);
    }

    virtual void onExit(event_type event) override
    {
        if (m_exitFunction)
            m_exitFunction(event);
    }

private:
    function_type m_entryFunction;
    function_type m_exitFunction;
};

} // namespace fsm11

#endif // FSM11_FUNCTIONSTATE_HPP
