/*******************************************************************************
  The MIT License (MIT)

  Copyright (c) 2015 Manuel Freiberger

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*******************************************************************************/

#ifndef FSM11_STATEMACHINE_HPP
#define FSM11_STATEMACHINE_HPP

#include "statemachine_fwd.hpp"
#include "options.hpp"
#include "state.hpp"
#include "transition.hpp"

#include "detail/callbacks.hpp"
#include "detail/capturestorage.hpp"
#include "detail/eventdispatcher.hpp"
#include "detail/multithreading.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/mutex.hpp>
#include <weos/type_traits.hpp>
#else
#include <mutex>
#include <type_traits>
#endif // FSM11_USE_WEOS

#include <iterator>

namespace fsm11
{
namespace fsm11_detail
{

#if 0
//! \brief A state machine visitor.
//!
//! The Visitor is the base class for all visitors which operate on a
//! state machine. It walks the state hierarchy in a depth-first manner
//! and can be used for implementing both pre-order and post-order traversal.
//!
//! For a pre-order traversal, the derived class has to implement one or
//! more of the <tt>visit*State()</tt> methods. For every state \p s, the
//! following sequence is executed:
//! \code
//! visitState(s);
//! if (s->isAtomic()) visitAtomicState(s);
//! if (!s->isAtomic()) visitNonAtomicState(s);
//! if (s->isCompound()) visitCompoundState(s);
//! if (s->isParallel()) visitParallelState(s);
//! \endcode
//!
//! The <tt>leave*State()</tt> methods are needed for a post-order traversal.
//! Their calling sequence is exactly the reverse of the corresponding
//! pre-order methods and so the following code is executed for every state
//! \p s:
//! \code
//! if (s->isParallel()) leaveParallelState(s);
//! if (s->isCompound()) leaveCompoundState(s);
//! if (!s->isAtomic()) leaveNonAtomicState(s);
//! if (s->isAtomic()) leaveAtomicState(s);
//! leaveState(s);
//! \endcode
template <typename TDerived>
class Visitor
{
public:
    //! \brief Applies the visitor to a state machine.
    //!
    //! Applies this visitor to the state machine \p machine.
    void operator() (StateMachine& machine);

    //! \brief Called from the state machine.
    //!
    //! This method is called from the state machine for every visited
    //! \p state. It dispatches the kind of the \p state and calls into
    //! the user-defined visit*State() methods.
    //! \internal
    void doVisit(state_type* state)
    {
        derived().visitState(state);
        if (state->isAtomic())
        {
            derived().visitAtomicState(state);
        }
        else
        {
            derived().visitNonAtomicState(state);
            if (state->isCompound())
                derived().visitCompoundState(state);
            else
                derived().visitParallelState(state);
        }
    }

    //! \brief Called from the state machine.
    //!
    //! This method is called from the state machine for every \p state
    //! to be left. It dispatches the kind of the \p state and calls into
    //! the user-defined leave*State() methods.
    //! \internal
    void doLeave(state_type* state)
    {
        if (state->isAtomic())
        {
            derived().leaveAtomicState(state);
        }
        else
        {
            if (state->isCompound())
                derived().leaveCompoundState(state);
            else
                derived().leaveParallelState(state);
            derived().leaveNonAtomicState(state);
        }
        derived().leaveState(state);
    }

    //! \brief Visits a state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given \p state.
    void visitState(state_type* /*state*/) {}

    //! \brief Visits an atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given atomic \p state.
    void visitAtomicState(state_type* /*state*/) {}

    //! \brief Visits a non-state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given non-atomic \p state.
    void visitNonAtomicState(state_type* /*state*/) {}

    //! \brief Visits a compound state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given compound \p state.
    void visitCompoundState(state_type* /*state*/) {}

    //! \brief Visits a parallel state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given parallel \p state.
    void visitParallelState(state_type* /*state*/) {}


    //! \brief Leaves a state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given \p state.
    void leaveState(state_type* /*state*/) {}

    //! \brief Leaves an atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given atomic \p state.
    void leaveAtomicState(state_type* /*state*/) {}

    //! \brief Leaves a non-atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given non-atomic \p state.
    void leaveNonAtomicState(state_type* /*state*/) {}

    //! \brief Leaves a compound state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given compound \p state.
    void leaveCompoundState(state_type* /*state*/) {}

    //! \brief Leaves a parallel state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given parallel \p state.
    void leaveParallelState(state_type* /*state*/) {}

private:
    //! Casts this instance to the derived class.
    TDerived& derived()
    {
        return *static_cast<TDerived*>(this);
    }
};

//! \brief A state machine configuration visitor.
//!
//! The ConfigurationVisitor is the base class for all visitors which operate
//! a state machine's configuration, i.e. on the set of active states of a
//! state machine. It walks the state hierarchy in a depth-first manner
//! and can be used for implementing both pre-order and post-order traversal.
//!
//! All the properties of a Visitor apply to the ConfigurationVisitor, too,
//! except that the callbacks are only invoked when the state is active.
template <typename TDerived>
class ConfigurationVisitor
{
public:
    //! \brief Applies the visitor to a state machine.
    //!
    //! Applies this visitor to the state machine \p machine.
    void operator() (StateMachine& machine);

    //! \brief Called from the state machine.
    //!
    //! This method is called from the state machine for every visited
    //! \p state. It dispatches the kind of the \p state and calls into
    //! the user-defined visit*State() methods.
    //! \internal
    void doVisit(state_type* state)
    {
        //assert(state->isActive());

        derived().visitState(state);
        if (state->isAtomic())
        {
            derived().visitAtomicState(state);
        }
        else
        {
            derived().visitNonAtomicState(state);
            if (state->isCompound())
                derived().visitCompoundState(state);
            else
                derived().visitParallelState(state);
        }
    }

    //! \brief Called from the state machine.
    //!
    //! This method is called from the state machine for every \p state
    //! to be left. It dispatches the kind of the \p state and calls into
    //! the user-defined leave*State() methods.
    //! \internal
    void doLeave(state_type* state)
    {
        //assert(state->isActive());

        if (state->isAtomic())
        {
            derived().leaveAtomicState(state);
        }
        else
        {
            if (state->isCompound())
                derived().leaveCompoundState(state);
            else
                derived().leaveParallelState(state);
            derived().leaveNonAtomicState(state);
        }
        derived().leaveState(state);
    }

    //! \brief Visits a state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given \p state.
    void visitState(state_type* /*state*/) {}

    //! \brief Visits an atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given atomic \p state.
    void visitAtomicState(state_type* /*state*/) {}

    //! \brief Visits a non-state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given non-atomic \p state.
    void visitNonAtomicState(state_type* /*state*/) {}

    //! \brief Visits a compound state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given compound \p state.
    void visitCompoundState(state_type* /*state*/) {}

    //! \brief Visits a parallel state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a pre-order traversal of the given parallel \p state.
    void visitParallelState(state_type* /*state*/) {}


    //! \brief Leaves a state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given \p state.
    void leaveState(state_type* /*state*/) {}

    //! \brief Leaves an atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given atomic \p state.
    void leaveAtomicState(state_type* /*state*/) {}

    //! \brief Leaves a non-atomic state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given non-atomic \p state.
    void leaveNonAtomicState(state_type* /*state*/) {}

    //! \brief Leaves a compound state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given compound \p state.
    void leaveCompoundState(state_type* /*state*/) {}

    //! \brief Leaves a parallel state.
    //!
    //! This method has to be overridden in a derived class in order to
    //! perform a post-order traversal of the given parallel \p state.
    void leaveParallelState(state_type* /*state*/) {}

private:
    //! Casts this instance to the derived class.
    TDerived& derived()
    {
        return *static_cast<TDerived*>(this);
    }
};
#endif










//! \brief A finite state machine.
//!
//! The StateMachine implements the logic for a finite state machine. The
//! state machine itself derives from State and so every state machine has
//! an implicit root state. All top-level states of a state machine, are
//! children of this root state.
template <typename TOptions>
class StateMachineImpl :
        public get_multithreading<TOptions>::type,
        public get_dispatcher<TOptions>::type,
        public get_configuration_change_callbacks<TOptions>::type,
        public get_event_callbacks<TOptions>::type,
        public get_state_callbacks<TOptions>::type,
        public get_state_exception_callbacks<TOptions>::type,
        public get_storage<TOptions>::type,
        public get_transition_conflict_callbacks<TOptions>::type,
        public State<StateMachineImpl<TOptions>>
{
public:
    using type = StateMachineImpl<TOptions>;
    using state_type = State<type>;
    using transition_type = Transition<type>;
    using event_type = typename TOptions::event_type;
    using event_list_type = typename TOptions::event_list_type;
    using transition_allocator_type = typename TOptions::transition_allocator_type;

private:
    using dispatcher_type = typename get_dispatcher<TOptions>::type;
    using storage_type = typename get_storage<TOptions>::type;
    using rebound_transition_allocator_t
        = typename transition_allocator_type::
          template rebind<transition_type>::other;

    friend dispatcher_type;
    friend storage_type;

public:
    StateMachineImpl()
        : state_type("(StateMachine)")
    {
        state_type::m_stateMachine = this;
    }

    explicit StateMachineImpl(const transition_allocator_type& alloc)
        : state_type("(StateMachine)"),
          m_transitionAllocator(alloc)
    {
        state_type::m_stateMachine = this;
    }

    //! \brief Destroys the state machine.
    virtual ~StateMachineImpl()
    {
        this->halt();

        for (auto& state : *this)
            state.deleteTransitions(m_transitionAllocator);
    }

    StateMachineImpl(const StateMachineImpl&) = delete;
    StateMachineImpl& operator=(const StateMachineImpl&) = delete;

//    template <typename TDerived>
//    void apply(Visitor<TDerived>& visitor);

//    template <typename TDerived>
//    void apply(Visitor<TDerived>&& visitor)
//    {
//        apply(visitor);
//    }

//    template <typename TDerived>
//    void apply(ConfigurationVisitor<TDerived>& visitor);

    //! \brief Adds a transition.
    //!
    //! Adds a transition, which will be created from a transition specification
    //! \p t.
    template <typename TState, typename TEvent, typename TGuard,
              typename TAction>
    void add(TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>&& t);

    //! \brief Adds a transition.
    //!
    //! Adds a transition, which will be created from a transition specification
    //! \p t.
    template <typename TState, typename TGuard, typename TAction>
    void add(TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction>&& t);

    //! \brief Adds a transition.
    template <typename TState, typename TEvent, typename TGuard,
              typename TAction>
    inline
    StateMachineImpl& operator+=(
            TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>&& t)
    {
        add(FSM11STD::move(t));
        return *this;
    }

    //! \brief Adds a transition.
    template <typename TState, typename TGuard, typename TAction>
    inline
    StateMachineImpl& operator+=(TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction>&& t)
    {
        add(FSM11STD::move(t));
        return *this;
    }

private:
    //! A list of events which have to be handled by the event loop.
    event_list_type m_eventList;
    //! The allocator for transitions.
    rebound_transition_allocator_t m_transitionAllocator;


    friend class EventDispatcherBase<StateMachineImpl>;
};

template <typename TOptions>
template <typename TState, typename TEvent, typename TGuard,
          typename TAction>
void StateMachineImpl<TOptions>::add(
        TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>&& t)
{
    // TODO: Use a unique_ptr here
    void* mem = m_transitionAllocator.allocate(1);
    transition_type* transition = new (mem) transition_type(FSM11STD::move(t));
    transition->source()->pushBackTransition(transition);
}

template <typename TOptions>
template <typename TState, typename TGuard, typename TAction>
void StateMachineImpl<TOptions>::add(
        TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction>&& t)
{
    // TODO: Use a unique_ptr here
    void* mem = m_transitionAllocator.allocate(1);
    transition_type* transition = new (mem) transition_type(FSM11STD::move(t));
    transition->source()->pushBackTransition(transition);
}




#if 0
template <typename TDerived>
void StateMachine::apply(Visitor<TDerived>& visitor)
{
    state_type* iterator = &m_rootState;
    while (1)
    {
        visitor.doVisit(iterator);
        if (iterator->m_children)
        {
            iterator = iterator->m_children;
        }
        else
        {
            visitor.doLeave(iterator);
            while (iterator->m_nextSibling == 0)
            {
                iterator = iterator->m_parent;
                if (iterator == 0)
                    return;
                visitor.doLeave(iterator);
            }
            iterator = iterator->m_nextSibling;
        }
    }
}

template <typename TDerived>
void Visitor<TDerived>::operator() (StateMachine& machine)
{
    machine.apply(*this);
}

template <typename TDerived>
void ConfigurationVisitor<TDerived>::operator() (StateMachine& machine)
{
    machine.apply(*this);
}
#endif

} // namespace fsm11_detail





template <typename... TOptions>
struct makeStateMachineType
{
private:
    using packed_options = typename fsm11_detail::pack_options<fsm11_detail::default_options,
                                                               TOptions...>::type;
public:
    using type = fsm11_detail::StateMachineImpl<packed_options>;
};



template <typename... TOptions>
using StateMachine = typename makeStateMachineType<TOptions...>::type;



#if DOXYGEN

template <typename... TOptions>
class Statemachine
{
public:
    //! The type of the state machine.
    using type = StateMachine<TOptions...>;
    using event_type = typename TOptions::event_type;
    using event_list_type = typename TOptions::event_list_type;
    using state_type = State<type>;
    using transition_type = Transition<TOptions>;


    //! Adds another \p event to the state machine.
    void addEvent(event_type event);

    //! Starts the state machine.
    void start();

    //! Starts the state machine.
    //!
    //! Starts the state machine. The attributes \p attrs are passed to the
    //! thread running the event loop.
    //!
    //! \note This overload is only available, if asynchronous event
    //! dispatching has been enabled.
    void start(const weos::thread::attributes& attrs);

    void stop();

    bool running() const;



    //! \note Calling this method directly is almost certainly a
    //! mistake. Instead an RAII-lock such as std::lock_guard<> or
    //! std::unique_lock<> should be used as in the following example:
    //! \code
    //! StateMachine sm;
    //! std::unique_lock<StateMachine> lock(sm);
    //! ... operate on the state machine ...
    //! \endcode
    void lock();

    //! \note Calling this method directly is almost certainly a mistake.
    //!
    //! \sa lock()
    void unlock();


    //! Loads an element from the storage.
    //!
    //! \p T is the type of the \p TIndex-th element in the storage.
    //!
    //! \note This function is only available, if a storage has been specified.
    template <std::size_t TIndex>
    const T& load() const;

    //! Sets an element in the storage.
    //!
    //! Sets the \p TIndex-th element in the storage to the given \p value.
    //!
    //! \note This function is only available, if a storage has been specified.
    template <std::size_t TIndex, typename TType>
    void store(TType&& value);
};

#endif // DOXYGEN

} // namespace fsm11

#endif // FSM11_STATEMACHINE_HPP
