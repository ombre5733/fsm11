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

#ifndef FSM11_DETAIL_EVENTDISPATCHER_HPP
#define FSM11_DETAIL_EVENTDISPATCHER_HPP

#include "../statemachine_fwd.hpp"
#include "scopeguard.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/atomic.hpp>
#include <weos/condition_variable.hpp>
#include <weos/future.hpp>
#include <weos/mutex.hpp>
#include <weos/utility.hpp>
#include <weos/thread.hpp>
#else
#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <utility>
#include <thread>
#endif // FSM11_USE_WEOS

namespace fsm11
{
namespace fsm11_detail
{

// ----=====================================================================----
//     EventDispatcherBase
// ----=====================================================================----

template <typename TDerived>
class EventDispatcherBase
{
public:
    EventDispatcherBase() noexcept
        : m_enabledTransitions(0),
          m_numConfigurationChanges(0)
    {
    }

    unsigned numConfigurationChanges() const noexcept
    {
        return m_numConfigurationChanges;
    }

protected:
    using options = typename get_options<TDerived>::type;
    using event_type = typename options::event_type;
    using state_type = State<TDerived>;
    using transition_type = Transition<TDerived>;


    //! The set of enabled transitions.
    transition_type* m_enabledTransitions;

    FSM11STD::atomic_uint m_numConfigurationChanges;


    TDerived& derived()
    {
        return *static_cast<TDerived*>(this);
    }

    const TDerived& derived() const
    {
        return *static_cast<const TDerived*>(this);
    }


    //! Clears the set of enabled transitions.
    void clearEnabledTransitionsSet() noexcept;

    //! \brief Selects matching transitions.
    //!
    //! Loops over all states and selects all transitions matching the given
    //! criteria. If \p onlyEventless is set, only transitions without events
    //! are selected. Otherwise, a transition is selected, if it's trigger event
    //! equals the given \p event.
    void selectTransitions(bool onlyEventless, event_type event);

    //! Computes the transition domain of the given \p transition.
    static state_type* transitionDomain(const transition_type* transition);

    //! Clears the transient flags of all states.
    void clearTransientStateFlags() noexcept;

    //! \brief Propagates the entry mark to all descendant states.
    //!
    //! Loops over all states in the state machine. If a state \p S is marked
    //! for entry, one of its children (in case \p S is a compound state) or
    //! all of its children (in case \p S is a parallel state) will be marked
    //! for entry, too.
    void markDescendantsForEntry();

    //! Enters all states in the enter-set.
    void enterStatesInEnterSet(event_type event);

    //! Leaves all states in the exit-set.
    void leaveStatesInExitSet(event_type event);

    //! \brief Performs a microstep.
    //!
    //! Performs a microstep. The given \p event is passed to the onEntry()
    //! and onExit() functions. The return value is \p true, if the
    //! configuration has been changed.
    bool microstep(event_type event);

    //! Follows all eventless transitions. Invokes the configuration change
    //! callback, if either \p changedConfiguration is set or at least one
    //! eventless transition has been triggered, which changed the
    //! configuration.
    void runToCompletion(bool changedConfiguration);

    //! Enters the initial states and thus brings up the state machine.
    void enterInitialStates();

    //! Leaves the current configuration, which effectively stops the
    //! state machine.
    void leaveConfiguration();
};

template <typename TDerived>
void EventDispatcherBase<TDerived>::clearEnabledTransitionsSet() noexcept
{
    auto transition = m_enabledTransitions;
    m_enabledTransitions = 0;
    while (transition != 0)
    {
        auto next = transition->m_nextInEnabledSet;
        transition->m_nextInEnabledSet = 0;
        transition = next;
    }
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::selectTransitions(bool onlyEventless,
                                                      event_type event)
{
    transition_type** outputIter = &m_enabledTransitions;

    // Loop over the states in post-order. This way, the descendent states are
    // checked before their ancestors.
    for (auto stateIter = derived().post_order_begin();
         stateIter != derived().post_order_end(); ++stateIter)
    {
        if (!(stateIter->m_flags & state_type::Active))
            continue;

        // If a transition in a descendant of a parallel state has already
        // been selected, the parallel state itself and all its ancestors
        // can be skipped.
        if (stateIter->m_flags & state_type::SkipTransitionSelection)
            continue;

        bool foundTransition = false;
        for (auto transitionIter = stateIter->beginTransitions();
             transitionIter != stateIter->endTransitions(); ++transitionIter)
        {
            // Skip transitions with events in microstepping mode.
            if (onlyEventless && !transitionIter->eventless())
                continue;

            // If a transition has an event, the event must match.
            if (!transitionIter->eventless()
                && transitionIter->event() != event)
            {
                continue;
            }

            // If the transition has a guard, it must evaluate to true in order
            // to select the transition. A transition without guard is selected
            // unconditionally.
            if (!transitionIter->guard() || transitionIter->guard()(event))
            {
                *outputIter = &*transitionIter;
                outputIter = &transitionIter->m_nextInEnabledSet;
                foundTransition = true;
                break;
            }
        }

        if (foundTransition)
        {
            // As we have found a transition in this state, there is no need to
            // check the ancestors for a matching transition.
            bool hasParallelAncestor = false;
            state_type* ancestor = stateIter->parent();
            while (ancestor)
            {
                ancestor->m_flags |= state_type::SkipTransitionSelection;
                hasParallelAncestor |= ancestor->isParallel();
                ancestor = ancestor->parent();
            }

            // If none of the ancestors is a parallel state, there is no
            // need to continue scanning the other states. This is because
            // the remaining active states are all ancestors of the current
            // state and no transition in an ancestor is more specific than
            // the one which has been selected right now.
            if (!hasParallelAncestor)
                return;
        }
    }
}

template <typename TDerived>
auto EventDispatcherBase<TDerived>::transitionDomain(
        const transition_type* transition) -> state_type*
{
    if (transition->isInternal()
        && transition->source()->isCompound()
        && isDescendant(transition->target(), transition->source()))
    {
        return transition->source();
    }

    return findLeastCommonProperAncestor(transition->source(),
                                         transition->target());
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::clearTransientStateFlags() noexcept
{
    for (auto iter = derived().begin(); iter != derived().end(); ++iter)
        iter->m_flags &= ~state_type::Transient;
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::markDescendantsForEntry()
{
    for (auto state = derived().begin(); state != derived().end(); ++state)
    {
        if (!(state->m_flags & state_type::InEnterSet))
        {
            state.skipChildren();
            continue;
        }

        if (state->isCompound())
        {
            // Exactly one state of a compound state has to be marked for entry.
            bool childMarked = false;
            for (auto child = state.child_begin();
                 child != state.child_end(); ++child)
            {
                if (child->m_flags & state_type::InEnterSet)
                {
                    childMarked = true;
                    break;
                }
            }

            if (!childMarked)
            {
                if (state_type* initialState = state->initialState())
                {
                    do
                    {
                        initialState->m_flags |= state_type::InEnterSet;
                        initialState = initialState->parent();
                    } while (initialState != &*state);
                }
                else
                {
                    state->m_children->m_flags |= state_type::InEnterSet;
                }
            }
        }
        else if (state->isParallel())
        {
            // All child states of a parallel state have to be marked for entry.
            for (auto child = state.child_begin();
                 child != state.child_end(); ++child)
            {
                child->m_flags |= state_type::InEnterSet;
            }
        }
    }
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::enterStatesInEnterSet(event_type event)
{
    for (auto iter = derived().begin(); iter != derived().end(); ++iter)
    {
        if ((iter->m_flags & state_type::InEnterSet)
            && !(iter->m_flags & state_type::Active))
        {
            derived().invokeStateEntryCallback(&*iter);
            try
            {
                iter->onEntry(event);
            }
            catch (...)
            {
                if (options::state_exception_callbacks_enable)
                    derived().invokeStateExceptionCallback(
                                FSM11STD::current_exception());
                else
                    throw;
            }
            iter->m_flags |= (state_type::Active | state_type::StartInvoke);
        }
    }
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::leaveStatesInExitSet(event_type event)
{
    for (auto iter = derived().post_order_begin();
         iter != derived().post_order_end(); ++iter)
    {
        if (iter->m_flags & state_type::InExitSet)
        {
            derived().invokeStateExitCallback(&*iter);
            if (iter->m_flags & state_type::Invoked)
            {
                iter->m_flags &= ~state_type::Invoked;
                FSM11STD::exception_ptr exc = iter->exitInvoke();
                if (exc)
                {
                    if (options::state_exception_callbacks_enable)
                        derived().invokeStateExceptionCallback(
                                    FSM11STD::current_exception());
                    else
                        FSM11STD::rethrow_exception(exc);
                }
            }
            iter->m_flags &= ~(state_type::Active | state_type::StartInvoke
                               | state_type::InExitSet);
            try
            {
                iter->onExit(event);
            }
            catch (...)
            {
                if (options::state_exception_callbacks_enable)
                    derived().invokeStateExceptionCallback(
                                FSM11STD::current_exception());
                else
                    throw;
            }
        }
    }
}

template <typename TDerived>
bool EventDispatcherBase<TDerived>::microstep(event_type event)
{
    bool changedConfiguration = false;

    // 1. Mark the states in the exit set for exit and the target state of the
    //    transition for entry.
    for (transition_type *prev = nullptr, *transition = m_enabledTransitions;
         transition != nullptr;
         prev = transition, transition = transition->m_nextInEnabledSet)
    {
        if (!transition->target())
            continue;

        changedConfiguration = true;

        state_type* domain = transitionDomain(transition);

        if (prev)
        {
            // Make sure that no state of the transition domain has been
            // marked for exit. Otherwise, two transitions have an
            // overlapping exit set, which means that the transitions
            // conflict.
            bool conflict = false;
            for (auto iter = ++domain->pre_order_begin();
                 iter != domain->pre_order_end(); ++iter)
            {
                if ((iter->m_flags & state_type::Active)
                    && (iter->m_flags & state_type::InExitSet))
                {
                    conflict = true;
                    break;
                }
            }

            // In case of a conflict, we simply ignore this transition but
            // keep the old ones.
            if (conflict)
            {
                prev->m_nextInEnabledSet = transition->m_nextInEnabledSet;
                transition->m_nextInEnabledSet = 0;
                transition = prev;
                continue;
            }
        }

        // As there is no conflict, we can set the exit mark for the states in
        // the transition domain.
        for (auto iter = ++domain->pre_order_begin();
             iter != domain->pre_order_end(); ++iter)
        {
            if ((iter->m_flags & state_type::Active))
                iter->m_flags |= state_type::InExitSet;
        }

        // Finally, mark the ancestors of the target for entry, too. Note that
        // we cannot mark the children right now, because another transition
        // can target one of this target's descendants.
        state_type* ancestor = transition->target();
        while (ancestor && !(ancestor->m_flags & state_type::InEnterSet))
        {
            ancestor->m_flags |= state_type::InEnterSet;
            ancestor = ancestor->parent();
        }
    }

    // 2. Propagate the entry mark to the children.
    markDescendantsForEntry();

    // 3. Leave the states in the exit set.
    leaveStatesInExitSet(event);

    // 4. Execute the transitions' actions.
    for (transition_type* transition = m_enabledTransitions;
         transition != nullptr;
         transition = transition->m_nextInEnabledSet)
    {
        if (transition->action())
            transition->action()(event);
    }

    // 5. Enter the states in the enter set.
    enterStatesInEnterSet(event);

    return changedConfiguration;
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::runToCompletion(bool changedConfiguration)
{
    // We are in microstepping mode: follow all eventless transitions.
    while (1)
    {
        clearTransientStateFlags();
        selectTransitions(true, event_type());
        if (!m_enabledTransitions)
            break;
        changedConfiguration |= microstep(event_type());
        clearEnabledTransitionsSet();
    }

    // Synchronize the visible state active flag with the internal
    // state active flag.
    for (auto iter = derived().begin(); iter != derived().end(); ++iter)
        iter->m_visibleActive = (iter->m_flags & state_type::Active) != 0;

    // Call the invoke() methods of all currently active states.
    for (auto iter = derived().begin(); iter != derived().end(); ++iter)
    {
        if (iter->m_flags & state_type::StartInvoke)
        {
            iter->enterInvoke();
            iter->m_flags &= ~state_type::StartInvoke;
            iter->m_flags |= state_type::Invoked;
        }
    }

    // If we followed at least one transition, which was not target-less,
    // invoke the configuration change callback.
    if (changedConfiguration)
    {
        ++m_numConfigurationChanges;
        derived().invokeConfigurationChangeCallback();
    }
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::enterInitialStates()
{
    // TODO: Would be nice, if the state machine had an initial
    // transition similar to initial transitions of states.
    clearTransientStateFlags();
    derived().m_flags |= state_type::InEnterSet;
    markDescendantsForEntry();
    enterStatesInEnterSet(event_type());
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::leaveConfiguration()
{
    for (auto iter = derived().begin(); iter != derived().end(); ++iter)
    {
        if (iter->m_flags & state_type::Active)
            iter->m_flags |= state_type::InExitSet;
    }
    leaveStatesInExitSet(event_type());

    for (auto iter = derived().begin(); iter != derived().end(); ++iter)
        iter->m_visibleActive = false;

    ++m_numConfigurationChanges;
    derived().invokeConfigurationChangeCallback();

    //! \todo Clear the event list? Or document that the event list is
    //! preserved when the FSM is stopped?
}

// ----=====================================================================----
//     SynchronousEventDispatcher
// ----=====================================================================----

template <typename TDerived>
class SynchronousEventDispatcher : public EventDispatcherBase<TDerived>
{
    using options = typename get_options<TDerived>::type;

public:
    using event_type = typename options::event_type;

    SynchronousEventDispatcher()
        : m_dispatching(false),
          m_running(false)
    {
    }

    void addEvent(event_type event)
    {
        auto lock = derived().getLock();

        derived().m_eventList.push_back(FSM11STD::move(event));
        doDispatchEvents();
    }

    bool running() const
    {
        auto lock = derived().getLock();
        return m_running;
    }

    void start()
    {
        auto lock = derived().getLock();
        if (!m_running)
        {
            FSM11_SCOPE_FAILURE {
                this->clearEnabledTransitionsSet();
                this->leaveConfiguration();
            };

            derived().invokeCaptureStorageCallback();
            this->enterInitialStates();
            this->runToCompletion(true);
            m_running = true;
            doDispatchEvents();
        }
    }

    void stop()
    {
        auto lock = derived().getLock();
        if (m_running)
        {
            m_running = false;
            FSM11_SCOPE_FAILURE { this->leaveConfiguration(); };
            derived().invokeCaptureStorageCallback();
            this->leaveConfiguration();
        }
    }

    template <typename T = void>
    void eventLoop()
    {
        static_assert(!FSM11STD::is_same<T, T>::value,
                      "A synchronous statemachine has no event-loop.");
    }

    template <typename T = void>
    void startAsyncEventLoop()
    {
        static_assert(!FSM11STD::is_same<T, T>::value,
                      "A synchronous statemachine has no event-loop.");
    }

protected:
    void halt()
    {
        stop();
    }

private:
    bool m_dispatching;
    bool m_running;

    TDerived& derived()
    {
        return *static_cast<TDerived*>(this);
    }

    const TDerived& derived() const
    {
        return *static_cast<const TDerived*>(this);
    }

    void doDispatchEvents()
    {
        if (!m_running || m_dispatching)
            return;

        m_dispatching = true;
        FSM11_SCOPE_EXIT { m_dispatching = false; };
        FSM11_SCOPE_FAILURE {
            this->clearEnabledTransitionsSet();
            this->leaveConfiguration();
            m_running = false;
        };

        while (!derived().m_eventList.empty())
        {
            auto event = derived().m_eventList.front();
            derived().m_eventList.pop_front();

            derived().invokeEventDispatchCallback(event);
            derived().invokeCaptureStorageCallback();

            this->clearTransientStateFlags();
            this->selectTransitions(false, event);
            bool changedConfiguration = false;
            if (this->m_enabledTransitions)
            {
                changedConfiguration = this->microstep(FSM11STD::move(event));
                this->clearEnabledTransitionsSet();
            }
            else
            {
                derived().invokeEventDiscardedCallback(FSM11STD::move(event));
            }

            this->runToCompletion(changedConfiguration);
        }
    }
};

template <typename TDerived>
class AsynchronousEventDispatcher : public EventDispatcherBase<TDerived>
{
    using options = typename get_options<TDerived>::type;

public:
    using event_type = typename options::event_type;

    AsynchronousEventDispatcher()
        : m_startRequest(false),
          m_stopRequest(false),
          m_eventLoopActive(false),
          m_running(false)
    {
    }

    AsynchronousEventDispatcher(const AsynchronousEventDispatcher&) = delete;
    AsynchronousEventDispatcher& operator=(const AsynchronousEventDispatcher&) = delete;

    void addEvent(event_type event)
    {
        {
            FSM11STD::lock_guard<FSM11STD::mutex> lock(m_eventLoopMutex);
            derived().m_eventList.push_back(FSM11STD::move(event));
        }

        m_continueEventLoop.notify_one();
    }

    bool running() const
    {
        auto lock = derived().getLock();
        return m_running;
    }

    void start()
    {
        m_eventLoopMutex.lock();
        m_startRequest = true;
        m_eventLoopMutex.unlock();
        m_continueEventLoop.notify_one();
    }

    void stop()
    {
        m_eventLoopMutex.lock();
        m_stopRequest = true;
        m_eventLoopMutex.unlock();
        m_continueEventLoop.notify_one();
    }

    void eventLoop()
    {
        {
            FSM11STD::lock_guard<FSM11STD::mutex> eventLoopLock(m_eventLoopMutex);
            // TODO: if (m_eventLoopRunning) throw;
            m_eventLoopActive = true;
        }

        doEventLoop();
    }

#ifdef FSM11_USE_WEOS
    FSM11STD::future<void> startAsyncEventLoop(const weos::thread::attributes& attrs)
    {
        FSM11STD::lock_guard<FSM11STD::mutex> eventLoopLock(m_eventLoopMutex);
        // TODO: if (m_eventLoopRunning) throw;
        m_eventLoopActive = true;
        return FSM11STD::async(attrs, &AsynchronousEventDispatcher::doEventLoop, this);
    }
#else
    FSM11STD::future<void> startAsyncEventLoop()
    {
        FSM11STD::lock_guard<FSM11STD::mutex> eventLoopLock(m_eventLoopMutex);
        // TODO: if (m_eventLoopRunning) throw;
        m_eventLoopActive = true;
        return FSM11STD::async(std::launch::async,
                               &AsynchronousEventDispatcher::doEventLoop, this);
    }
#endif

protected:
    void halt()
    {
        stop();
        FSM11STD::unique_lock<FSM11STD::mutex> eventLoopLock(m_eventLoopMutex);
        m_continueEventLoop.wait(eventLoopLock,
                                 [this]{ return !m_eventLoopActive; });
    }

private:
    //! A mutex to prevent concurrent modifications of the request flags.
    mutable FSM11STD::mutex m_eventLoopMutex;
    //! This CV signals that a new control event is available.
    FSM11STD::condition_variable m_continueEventLoop;
    //! Set if starting the state machine has been requested.
    bool m_startRequest;
    //! Set if stopping the state machine has been requested.
    bool m_stopRequest;
    //! Set if the event loop is running.
    bool m_eventLoopActive;

    //! Set if the state machine is running. Guarded by the multithreading
    //! lock but not by m_eventLoopMutex.
    bool m_running;

    TDerived& derived()
    {
        return *static_cast<TDerived*>(this);
    }

    const TDerived& derived() const
    {
        return *static_cast<const TDerived*>(this);
    }

    void doEventLoop()
    {
        FSM11_SCOPE_EXIT {
            m_eventLoopMutex.lock();
            m_eventLoopActive = false;
            m_eventLoopMutex.unlock();
            m_continueEventLoop.notify_all();
        };

        do
        {
            {
                // Wait until a start or stop request has been sent.
                FSM11STD::unique_lock<FSM11STD::mutex> eventLoopLock(m_eventLoopMutex);
                m_continueEventLoop.wait(eventLoopLock,
                                         [this]{ return m_startRequest || m_stopRequest; });
                m_startRequest = false;
                if (m_stopRequest)
                {
                    m_stopRequest = false;
                    return;
                }
                eventLoopLock.unlock();

                auto lock = derived().getLock();
                FSM11_SCOPE_FAILURE {
                    this->clearEnabledTransitionsSet();
                    this->leaveConfiguration();
                };

                derived().invokeCaptureStorageCallback();
                this->enterInitialStates();
                this->runToCompletion(true);
                m_running = true;
            }

            while (true)
            {
                typename options::event_type event;

                // Wait until either an event is added to the list or
                // an FSM stop has been requested.
                FSM11STD::unique_lock<FSM11STD::mutex> eventLoopLock(m_eventLoopMutex);
                m_continueEventLoop.wait(
                            eventLoopLock,
                            [this]{ return !derived().m_eventList.empty() || m_stopRequest; });
                m_startRequest = false;
                if (m_stopRequest)
                {
                    m_stopRequest = false;
                    auto lock = derived().getLock();
                    m_running = false;
                    FSM11_SCOPE_FAILURE { this->leaveConfiguration(); };
                    derived().invokeCaptureStorageCallback();
                    this->leaveConfiguration();
                    break;
                }

                // Get the next event from the event list.
                event = derived().m_eventList.front();
                derived().m_eventList.pop_front(); // TODO: What if this throws?
                eventLoopLock.unlock();

                auto lock = derived().getLock();
                FSM11_SCOPE_FAILURE {
                    m_running = false;
                    this->clearEnabledTransitionsSet();
                    this->leaveConfiguration();
                };

                derived().invokeEventDispatchCallback(event);
                derived().invokeCaptureStorageCallback();

                this->clearTransientStateFlags();
                this->selectTransitions(false, event);
                bool changedConfiguration = false;
                if (this->m_enabledTransitions)
                {
                    changedConfiguration
                            = this->microstep(FSM11STD::move(event));
                    this->clearEnabledTransitionsSet();
                }
                else
                {
                    derived().invokeEventDiscardedCallback(FSM11STD::move(event));
                }

                this->runToCompletion(changedConfiguration);
            }
        } while (false); // TODO: have an option to continue looping even after a stop request
    }
};

template <bool TSynchronous, typename TOptions>
struct get_dispatcher_helper
{
    typedef SynchronousEventDispatcher<StateMachineImpl<TOptions>> type;
};

template <typename TOptions>
struct get_dispatcher_helper<false, TOptions>
{
    typedef AsynchronousEventDispatcher<StateMachineImpl<TOptions>> type;
};

template <typename TOptions>
struct get_dispatcher
        : public get_dispatcher_helper<TOptions::synchronous_dispatch, TOptions>
{
};

} // namespace fsm11_detail
} // namespace fsm11

#endif // FSM11_DETAIL_EVENTDISPATCHER_HPP
