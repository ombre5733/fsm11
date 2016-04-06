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

#ifndef FSM11_STATEMACHINE_HPP
#define FSM11_STATEMACHINE_HPP

#include "statemachine_fwd.hpp"
#include "options.hpp"
#include "state.hpp"
#include "transition.hpp"

#include "detail/callbacks.hpp"
#include "detail/eventdispatcher.hpp"
#include "detail/meta.hpp"
#include "detail/multithreading.hpp"
#include "detail/threadpool.hpp"

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







// ----=====================================================================----
//     StateMachineImpl
// ----=====================================================================----


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
        public get_threadpool<TOptions>::type,
        public get_transition_conflict_action<TOptions>::type,
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
    using rebound_transition_allocator_t
        = typename transition_allocator_type::
          template rebind<transition_type>::other;
    using internal_thread_pool_type
        = typename get_threadpool<TOptions>::type::internal_thread_pool_type;

    friend dispatcher_type;

public:
    StateMachineImpl()
        : state_type("(StateMachine)")
    {
        state_type::m_stateMachine = this;
    }

    template <typename T = void,
              typename = typename std::enable_if<
                             TOptions::threadpool_enable, T>::type>
    explicit
    StateMachineImpl(internal_thread_pool_type&& pool)
        : state_type("(StateMachine)"),
          m_threadPool(std::move(pool))
    {
        state_type::m_stateMachine = this;
    }

    explicit
    StateMachineImpl(const transition_allocator_type& alloc)
        : state_type("(StateMachine)"),
          m_transitionAllocator(alloc)
    {
        state_type::m_stateMachine = this;
    }

    //! \brief Destroys the state machine.
    virtual
    ~StateMachineImpl()
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
    transition_type* add(TypeSourceEventGuardActionTarget<
                             TState, TEvent, TGuard, TAction>&& t);

    //! \brief Adds a transition.
    //!
    //! Adds a transition, which will be created from a transition specification
    //! \p t.
    template <typename TState, typename TGuard, typename TAction>
    transition_type* add(TypeSourceNoEventGuardActionTarget<
                             TState, TGuard, TAction>&& t);

    //! \brief Adds a transition.
    template <typename TState, typename TEvent, typename TGuard,
              typename TAction>
    inline
    transition_type* operator+=(
            TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>&& t)
    {
        return add(std::move(t));
    }

    //! \brief Adds a transition.
    template <typename TState, typename TGuard, typename TAction>
    inline
    transition_type* operator+=(TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction>&& t)
    {
        return add(std::move(t));
    }

    //! \brief Checks if the state machine is active.
    //!
    //! Returns \p true, if the state machine is active, which means that
    //! at least one of its child states is active, too.
    bool isActive() const noexcept;

    //! \brief Checks if a state is active.
    //!
    //! Returns \p true, if the \p state is active.
    bool isActive(const state_type& state) const noexcept;

    //! \brief Checks if at least one state is active.
    //!
    //! Returns \p true, if any state of the given list (\p state, \p states)
    //! is active, which means that the state belongs to the current state
    //! machine configuration.
    template <typename... TStates>
    bool isAnyActive(const state_type& state, const TStates&... states) const noexcept;

    //! \brief Checks if all states are active.
    //!
    //! Returns \p true, if all states of the given list (\p state, \p states)
    //! are active, which means that they all belong to the current state
    //! machine configuration.
    template <typename... TStates>
    bool areAllActive(const state_type& state, const TStates&... states) const noexcept;

private:
    //! A list of events which have to be handled by the event loop.
    event_list_type m_eventList;
    //! The allocator for transitions.
    rebound_transition_allocator_t m_transitionAllocator;

    internal_thread_pool_type m_threadPool; // TODO: merge if empty


    internal_thread_pool_type& threadPool()
    {
        return m_threadPool;
    }



    template <typename... TStates>
    bool doAnyActive(const state_type& state, const TStates&... states) const noexcept
    {
        return (state.m_flags & state_type::VisibleActive)
                ? true
                : doAnyActive(states...);
    }

    template <typename... TStates>
    bool doAnyActive() const noexcept
    {
        return false;
    }

    template <typename... TStates>
    bool doAllActive(const state_type& state, const TStates&... states) const noexcept
    {
        return !(state.m_flags & state_type::VisibleActive)
                ? false
                : doAllActive(states...);
    }

    template <typename... TStates>
    bool doAllActive() const noexcept
    {
        return true;
    }


    friend class EventDispatcherBase<StateMachineImpl>;

    template <typename T>
    friend class fsm11::ThreadedState;

    template <typename T>
    friend class WithThreadPool;
};

template <typename TOptions>
template <typename TState, typename TEvent, typename TGuard,
          typename TAction>
auto StateMachineImpl<TOptions>::add(
        TypeSourceEventGuardActionTarget<TState, TEvent, TGuard, TAction>&& t)
    -> transition_type*
{
    // TODO: Use a unique_ptr here
    void* mem = m_transitionAllocator.allocate(1);
    transition_type* transition = new (mem) transition_type(std::move(t));
    transition->source()->pushBackTransition(transition);
    return transition;
}

template <typename TOptions>
template <typename TState, typename TGuard, typename TAction>
auto StateMachineImpl<TOptions>::add(
        TypeSourceNoEventGuardActionTarget<TState, TGuard, TAction>&& t)
    -> transition_type*
{
    // TODO: Use a unique_ptr here
    void* mem = m_transitionAllocator.allocate(1);
    transition_type* transition = new (mem) transition_type(std::move(t));
    transition->source()->pushBackTransition(transition);
    return transition;
}

template <typename TOptions>
bool StateMachineImpl<TOptions>::isActive() const noexcept
{
    this->acquireStateActiveFlags();
    bool active = this->m_flags & state_type::VisibleActive;
    this->releaseStateActiveFlags();
    return active;
}

template <typename TOptions>
bool StateMachineImpl<TOptions>::isActive(const state_type& state) const noexcept
{
    this->acquireStateActiveFlags();
    bool active = state.m_flags & state_type::VisibleActive;
    this->releaseStateActiveFlags();
    return active;
}

template <typename TOptions>
template <typename... TStates>
bool StateMachineImpl<TOptions>::isAnyActive(const state_type& state,
                                             const TStates&... states) const noexcept
{
    using namespace std;

    static_assert(fsm11_detail::all<
                      is_base_of<state_type, TStates>::value...
                  >::value,
                  "All arguments have to be states");

    this->acquireStateActiveFlags();
    auto any = doAnyActive(state, states...);
    this->releaseStateActiveFlags();
    return any;
}

template <typename TOptions>
template <typename... TStates>
bool StateMachineImpl<TOptions>::areAllActive(const state_type& state,
                                              const TStates&... states) const noexcept
{
    using namespace std;

    static_assert(fsm11_detail::all<
                      is_base_of<state_type, TStates>::value...
                  >::value,
                  "All arguments have to be states");

    this->acquireStateActiveFlags();
    auto all = doAllActive(state, states...);
    this->releaseStateActiveFlags();
    return all;
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
            while (iterator->m_nextSibling == nullptr)
            {
                iterator = iterator->m_parent;
                if (iterator == nullptr)
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




// A helper to create the concrete state machine type.
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
};

#endif // DOXYGEN

} // namespace fsm11

#endif // FSM11_STATEMACHINE_HPP
