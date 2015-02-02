#ifndef FSM11_DETAIL_EVENTDISPATCHER_HPP
#define FSM11_DETAIL_EVENTDISPATCHER_HPP

#include "statemachine_fwd.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <utility>
#include <thread>

namespace fsm11
{
namespace detail
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
    using state_type = State<options>;
    using transition_type = Transition<options>;


    //! The set of enabled transitions.
    transition_type* m_enabledTransitions;

    std::atomic_uint m_numConfigurationChanges;


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
    //! criteria. If \p eventless is set, only transitions without events are
    //! selected. Otherwise, a transition is selected, if it's trigger event
    //! equals the given \p event.
    void selectTransitions(bool eventless, event_type event);

    //! Computes the transition domain of the given \p transition.
    static state_type* transitionDomain(const transition_type* transition);

    //! Clears the flags of all states.
    void clearStateFlags() noexcept;

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

    //! Performs a microstep.
    void microstep(event_type event);

    //! Follows all eventless transitions. Invokes the configuration change
    //! callback, if either followedTransition is set or at least one
    //! eventless transition triggered.
    void runToCompletion(bool followedTransition);

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
void EventDispatcherBase<TDerived>::selectTransitions(bool eventless, event_type event)
{
    transition_type** outputIter = &m_enabledTransitions;

    // Loop over the states in post-order. This way, the descendent states are
    // checked before their ancestors.
    for (auto stateIter = derived().post_order_begin();
         stateIter != derived().post_order_end(); ++stateIter)
    {
        if (!stateIter->m_internalActive)
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
            if (eventless != transitionIter->eventless())
                continue;

            if (!eventless && transitionIter->event() != event)
                continue;

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
auto EventDispatcherBase<TDerived>::transitionDomain(const transition_type* transition) -> state_type*
{
    //if (t.isInternal() && t->source().isCompound() && isDescendant(t->target(), t->source()))
    //    return t.source();
    return findLeastCommonProperAncestor(transition->source(),
                                         transition->target());
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::clearStateFlags() noexcept
{
    for (auto iter = derived().begin(); iter != derived().end(); ++iter)
        iter->m_flags = 0;
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
                //! \todo Add the possibility to have an initial state
                state->m_children->m_flags |= state_type::InEnterSet;
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
            && !iter->m_internalActive)
        {
            derived().invokeStateEntryCallback(&*iter);
            iter->onEntry(event);
            iter->m_internalActive = true;
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
            iter->m_internalActive = false;
            iter->exitInvoke();
            iter->onExit(event);
        }
    }
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::microstep(event_type event)
{
    // 1. Mark the states in the exit set for exit and the target state of the
    //    transition for entry.
    for (transition_type *prev = 0, *transition = m_enabledTransitions;
         transition != 0;
         prev = transition, transition = transition->m_nextInEnabledSet)
    {
        if (!transition->target())
            continue;

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
                if (iter->m_internalActive
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
            if (iter->m_internalActive)
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
         transition != 0;
         transition = transition->m_nextInEnabledSet)
    {
        if (transition->action())
            transition->action()(event);
    }

    // 5. Enter the states in the enter set.
    enterStatesInEnterSet(event);
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::runToCompletion(bool followedTransition)
{
    // We are in microstepping mode: follow all eventless transitions.
    while (1)
    {
        clearStateFlags();
        //WEOS_ON_SCOPE_EXIT(&StateMachine::clearEnabledTransitionsSet, this);
        selectTransitions(true, event_type());
        if (!m_enabledTransitions)
            break;
        followedTransition = true;
        microstep(event_type());
        clearEnabledTransitionsSet(); // TODO: replace this by the scope guard
    }

    // Synchronize the visible state active flag with the internal
    // state active flag.
    for (auto iter = derived().begin(); iter != derived().end(); ++iter)
        iter->m_visibleActive = iter->m_internalActive;

    // Call the invoke() methods of all currently active states.
    for (auto iter = derived().begin(); iter != derived().end(); ++iter)
    {
        if (iter->m_internalActive)
            iter->enterInvoke();
    }

    // If we followed at least one transition, invoke the configuration
    // change callback.
    if (followedTransition)
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
    clearStateFlags();
    derived().rootState()->m_flags |= state_type::InEnterSet;
    markDescendantsForEntry();
    enterStatesInEnterSet(event_type());
}

template <typename TDerived>
void EventDispatcherBase<TDerived>::leaveConfiguration()
{
    for (auto iter = derived().begin(); iter != derived().end(); ++iter)
    {
        if (iter->m_internalActive)
            iter->m_flags |= state_type::InExitSet;
    }
    leaveStatesInExitSet(event_type());
#if 0
    broadcastConfigurationChange();
#endif
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

    struct Holder
    {
        Holder(bool& b)
            : m_b(b)
        {
            m_b = true;
        }

        ~Holder()
        {
            m_b = false;
        }

        bool& m_b;
    };

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

        derived().m_eventList.push_back(std::move(event));
        if (!m_running || m_dispatching)
            return;

        Holder h(m_dispatching);
        while (!derived().m_eventList.empty())
        {
            //weos::ScopeGuard leaveGuard
            //        = weos::makeScopeGuard(&StateMachine::leaveConfiguration, this);

            auto event = derived().m_eventList.front();
            derived().m_eventList.pop_front();
            derived().invokeEventDispatchCallback(event);

            this->clearStateFlags();
            //WEOS_ON_SCOPE_EXIT(&StateMachine::clearEnabledTransitionsSet, this);
            this->selectTransitions(false, event);
            bool followedTransition = false;
            if (this->m_enabledTransitions)
            {
                followedTransition = true;
                this->microstep(std::move(event));
            }
            else
            {
                derived().invokeEventDiscardedCallback(std::move(event));
            }
            this->clearEnabledTransitionsSet();

            this->runToCompletion(followedTransition);

            //leaveGuard.dismiss();
        }
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
            this->enterInitialStates();
            this->runToCompletion(true);
            m_running = true;
        }
    }

    void stop()
    {
        auto lock = derived().getLock();
        // TODO
        m_running = false;
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
};

template <typename TDerived>
class AsynchronousEventDispatcher : public EventDispatcherBase<TDerived>
{
    using options = typename get_options<TDerived>::type;

public:
    using event_type = typename options::event_type;

    AsynchronousEventDispatcher()
        : m_stopRequest(false)
    {
    }

    AsynchronousEventDispatcher(const AsynchronousEventDispatcher&) = delete;
    AsynchronousEventDispatcher& operator=(const AsynchronousEventDispatcher&) = delete;

    void addEvent(event_type event)
    {
        {
            std::lock_guard<std::mutex> lock(m_eventLoopMutex);
            derived().m_eventList.push_back(std::move(event));
        }

        m_continueEventLoop.notify_one();
    }

    bool running() const
    {
        auto lock = derived().getLock();
        return m_eventLoopThread.joinable();
    }

    void start()
    {
        start(0);
    }

    void start(int /*attrs*/)
    {
        auto lock = derived().getLock();
        if (!m_eventLoopThread.joinable())
        {
            m_stopRequest = false;
            m_eventLoopThread = std::thread(
                                    // TODO: attrs
                                    &AsynchronousEventDispatcher::eventLoop,
                                    this);
        }
    }

    void stop()
    {
        auto lock = derived().getLock();
        if (m_eventLoopThread.joinable())
        {
            m_eventLoopMutex.lock();
            m_stopRequest = true;
            m_eventLoopMutex.unlock();
            m_continueEventLoop.notify_one();
            m_eventLoopThread.join();
        }
    }

private:
    //! A handle to the thread which dispatches the events.
    std::thread m_eventLoopThread;
    //! A mutex to suppress concurrent modifications to the thread handle.
    mutable std::mutex m_eventLoopMutex;

    //! A control event to steer the event loop.
    bool m_stopRequest;
    //! This CV signals that a new control event is available.
    std::condition_variable m_continueEventLoop;


    TDerived& derived()
    {
        return *static_cast<TDerived*>(this);
    }

    const TDerived& derived() const
    {
        return *static_cast<const TDerived*>(this);
    }

    void eventLoop()
    {
        {
            auto lock = derived().getLock();
            this->enterInitialStates();
            this->runToCompletion();
        }

        while (true)
        {
            //weos::ScopeGuard leaveGuard
            //        = weos::makeScopeGuard(&StateMachine::leaveConfiguration, this);

            typename options::event_type event;
            {
                // Wait until either an event is added to the list or
                // an FSM stop has been requested.
                std::unique_lock<std::mutex> lock(m_eventLoopMutex);
                m_continueEventLoop.wait(
                            lock, [this]{ return !derived().m_eventList.empty() || m_stopRequest; });
                if (m_stopRequest)
                    return;
                // Get the next event from the event list.
                event = derived().m_eventList.front();
                derived().m_eventList.pop_front();
            }

            auto lock = derived().getLock();
            derived().invokeEventDispatchCallback(event);

            this->clearStateFlags();
            //WEOS_ON_SCOPE_EXIT(&StateMachine::clearEnabledTransitionsSet, this);
            this->selectTransitions(false, event);
            bool followedTransition = false;
            if (this->m_enabledTransitions)
            {
                followedTransition = true;
                this->microstep(std::move(event));
            }
            else
            {
                derived().invokeEventDiscardedCallback(std::move(event));
            }
            this->clearEnabledTransitionsSet();

            this->runToCompletion(followedTransition);

            //leaveGuard.dismiss();
        }
    }
};

template <bool TSynchronous, typename TOptions>
struct get_dispatcher_helper
{
    typedef SynchronousEventDispatcher<StateMachine<TOptions>> type;
};

template <typename TOptions>
struct get_dispatcher_helper<false, TOptions>
{
    typedef AsynchronousEventDispatcher<StateMachine<TOptions>> type;
};

template <typename TOptions>
struct get_dispatcher
        : public get_dispatcher_helper<TOptions::synchronous_dispatch, TOptions>
{
};

} // namespace detail
} // namespace fsm11

#endif // FSM11_DETAIL_EVENTDISPATCHER_HPP
