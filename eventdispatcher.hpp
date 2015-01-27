#ifndef STATEMACHINE_EVENTDISPATCHER_HPP
#define STATEMACHINE_EVENTDISPATCHER_HPP

#include "statemachine_fwd.hpp"

#include <condition_variable>
#include <thread>

namespace statemachine
{
namespace detail
{

template <typename TDerived>
class EventDispatcherBase
{
protected:
    using options = typename get_options<TDerived>::type;
    using event_type = typename options::event_type;
    using state_type = State<options>;

    TDerived& derived()
    {
        return *static_cast<TDerived*>(this);
    }

    const TDerived& derived() const
    {
        return *static_cast<const TDerived*>(this);
    }

    void enterInitialStates()
    {
        // TODO: Would be nice, if the state machine had an initial
        // transition similar to initial transitions of states.
        derived().clearStateFlags();
        derived().rootState()->m_flags |= state_type::InEnterSet;
        derived().markDescendantsForEntry();
        derived().enterStatesInEnterSet(event_type());
    }

    void runToCompletion()
    {
        // We are in microstepping mode: follow all eventless transitions.
        while (1)
        {
            derived().clearStateFlags();
            //WEOS_ON_SCOPE_EXIT(&StateMachine::clearEnabledTransitionsSet, this);
            derived().selectTransitions(true, event_type());
            if (!derived().m_enabledTransitions)
                break;
            derived().microstep(event_type());
        }

        // Synchronize the visible state active flag with the internal
        // state active flag. Call the invoke() methods of all currently
        // active states.
        for (auto iter = derived().begin(); iter != derived().end(); ++iter)
        {
            iter->m_visibleActive = iter->m_internalActive;
            if (iter->m_visibleActive)
                iter->enterInvoke();
        }

#if 0
        // Notify all waiters about the configuration change.
        derived().broadcastConfigurationChange();
#endif
    }
};

template <typename TDerived>
class SynchronousEventDispatcher : public EventDispatcherBase<TDerived>
{
    enum FsmState
    {
        Stopped,
        Running
    };

public:
    SynchronousEventDispatcher()
        : m_fsmState(Stopped)
    {
    }

    //void addEvent(event_type event)

    bool running() const
    {
        return m_fsmState == Running;
    }

#if 0
    void start()
    {
        runToCompletion();
    }

    void addEvent(event_type event)
    {
        //weos::ScopeGuard leaveGuard
        //        = weos::makeScopeGuard(&StateMachine::leaveConfiguration, this);

        event_type event = m_eventList.front();
        m_eventList.pop_front();

        clearStateFlags();
        //WEOS_ON_SCOPE_EXIT(&StateMachine::clearEnabledTransitionsSet, this);
        selectTransitions(event);
        if (m_enabledTransitions)
            microstep(event);

        runToCompletion();

        //leaveGuard.dismiss();
    }

    void runToCompletion()
    {
        //weos::ScopeGuard leaveGuard
        //        = weos::makeScopeGuard(&StateMachine::leaveConfiguration, this);

        // We are in microstepping mode: follow all eventless transitions.
        while (1)
        {
            clearStateFlags();
            //WEOS_ON_SCOPE_EXIT(&StateMachine::clearEnabledTransitionsSet, this);
            selectTransitions(0);
            if (!m_enabledTransitions)
                break;
            microstep(0);
        }

        // Call the invoke() methods of all currently active states.
        for (pre_order_iterator iter = begin(); iter != end(); ++iter)
            iter->enterInvoke();

        // Notify all waiters about the configuration change.
        broadcastConfigurationChange();


        //leaveGuard.dismiss();
    }
#endif

private:
    FsmState m_fsmState;
};

template <typename TDerived>
class AsynchronousEventDispatcher : public EventDispatcherBase<TDerived>
{
    enum ControlEvent
    {
        None,
        StopRequest,
        EventAdded
    };

    using options = typename get_options<TDerived>::type;

public:
    using event_type = typename options::event_type;

    AsynchronousEventDispatcher()
        : m_controlEvent(None)
    {
    }

    AsynchronousEventDispatcher(const AsynchronousEventDispatcher&) = delete;
    AsynchronousEventDispatcher& operator=(const AsynchronousEventDispatcher&) = delete;

    void addEvent(event_type event)
    {
        {
            std::unique_lock<std::mutex> lock(derived().m_mutex);
            derived().m_eventList.push_back(std::move(event));
            m_controlEvent = EventAdded;
        }
        m_controlEventReceived.notify_one();
    }

    bool running() const
    {
        std::unique_lock<std::mutex> lock(derived().m_mutex);
        return m_eventLoopThread.joinable();
    }

    void start()
    {
        std::unique_lock<std::mutex> lock(derived().m_mutex);
        if (!m_eventLoopThread.joinable())
            m_eventLoopThread = std::thread(
                                    &AsynchronousEventDispatcher::eventLoop,
                                    this);
    }

    void stop()
    {
        //std::unique_lock<std::mutex> lock(derived().m_mutex);
        if (m_eventLoopThread.joinable())
        {
            derived().m_mutex.lock();
            m_controlEvent = StopRequest;
            derived().m_mutex.unlock();
            m_controlEventReceived.notify_one();
            m_eventLoopThread.join();
        }
    }

private:
    //! A handle to the thread which dispatches the events.
    std::thread m_eventLoopThread;

    //! A control event to steer the event loop.
    ControlEvent m_controlEvent;
    //! This CV signals that a new control event is available.
    std::condition_variable m_controlEventReceived;


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
            std::unique_lock<std::mutex> lock(derived().m_mutex);
            this->enterInitialStates();
            this->runToCompletion();
        }

        while (true)
        {
            std::unique_lock<std::mutex> lock(derived().m_mutex);
            //weos::ScopeGuard leaveGuard
            //        = weos::makeScopeGuard(&StateMachine::leaveConfiguration, this);

            // Wait for another event.
            m_controlEventReceived.wait(
                        lock, [&]{ return m_controlEvent != None; });
            if (m_controlEvent == StopRequest)
                return;
            m_controlEvent = None;

            // Get the next event from the event list.
            if (derived().m_eventList.empty())
                continue;
            auto event = derived().m_eventList.front();
            derived().m_eventList.pop_front();

            derived().clearStateFlags();
            //WEOS_ON_SCOPE_EXIT(&StateMachine::clearEnabledTransitionsSet, this);
            derived().selectTransitions(false, event);
            if (derived().m_enabledTransitions)
                derived().microstep(std::move(event));

            this->runToCompletion();

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
} // namespace statemachine

#endif // STATEMACHINE_EVENTDISPATCHER_HPP
