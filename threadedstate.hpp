#ifndef FSM11_THREADEDSTATE_HPP
#define FSM11_THREADEDSTATE_HPP

#include "statemachine_fwd.hpp"
#include "state.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/chrono.hpp>
#include <weos/condition_variable.hpp>
#include <weos/exception.hpp>
#include <weos/mutex.hpp>
#include <weos/thread.hpp>
#include <weos/type_traits.hpp>
#else
#include <chrono>
#include <condition_variable>
#include <exception>
#include <mutex>
#include <thread>
#include <type_traits>
#endif // FSM11_USE_WEOS

namespace fsm11
{

//! \brief A state with a threaded invoke action.
template <typename TStateMachine>
class ThreadedState : public State<TStateMachine>
{
    using base_type = State<TStateMachine>;

public:
    using type = ThreadedState<TStateMachine>;

    //! \brief Creates a state with a threaded invoke action.
    explicit ThreadedState(const char* name,
                           base_type* parent = 0)
        : base_type(name, parent)
    {
    }

#ifdef FSM11_USE_WEOS
    //! \brief Creates a state with a threaded invoke action.
    explicit ThreadedState(const char* name,
                           weos::thread::attributes attrs,
                           base_type* parent = 0)
        : base_type(name, parent),
          m_invokeThreadAttributes(attrs)
    {
    }
#endif // FSM11_USE_WEOS

    virtual ~ThreadedState()
    {
        // If the invoked thread is joinable, exitInvoke() has not been
        // called. This is the case if the state is destructed before the
        // state machine is stopped.
        assert(!m_invokeThread.joinable());
    }

    bool exitRequested() const
    {
        FSM11STD::lock_guard<FSM11STD::mutex> lock(m_mutex);
        return m_exitRequested;
    }

    void waitForExitRequest()
    {
        FSM11STD::unique_lock<FSM11STD::mutex> lock(m_mutex);
        m_cv.wait(lock, [&] { return m_exitRequested; });
    }

    template <typename TRep, typename TPeriod>
    bool waitForExitRequestFor(
            const FSM11STD::chrono::duration<TRep, TPeriod>& timeout)
    {
        FSM11STD::unique_lock<FSM11STD::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, timeout, [&] { return m_exitRequested; });
    }

    //! \brief The actual invoke action.
    //!
    //! This method is called in a new thread. Derived classes have to
    //! override this function.
    virtual void invoke() = 0;

    //! Enteres the invoked thread.
    virtual void enterInvoke() override
    {
        m_mutex.lock();
        m_exitRequested = false;
        m_mutex.unlock();

        m_exceptionPointer = nullptr;
        m_invokeThread = FSM11STD::thread(
#ifdef FSM11_USE_WEOS
                             m_invokeThreadAttributes,
#endif // FSM11_USE_WEOS
                             &ThreadedState::invokeWrapper, this);
    }

    //! Leaves the invoked thread.
    //!
    //! Joins with the thread in which the invoked action is running.
    virtual FSM11STD::exception_ptr exitInvoke() override
    {
        assert(m_invokeThread.joinable());

        m_mutex.lock();
        m_exitRequested = true;
        m_mutex.unlock();
        m_cv.notify_one();

        m_invokeThread.join();
        return m_exceptionPointer;
    }

private:
#ifdef FSM11_USE_WEOS
    weos::thread::attributes m_invokeThreadAttributes;
#endif // FSM11_USE_WEOS

    FSM11STD::thread m_invokeThread;
    FSM11STD::exception_ptr m_exceptionPointer;

    mutable FSM11STD::mutex m_mutex;
    FSM11STD::condition_variable m_cv;
    bool m_exitRequested;

    void invokeWrapper()
    {
        try
        {
            invoke();
        }
        catch (...)
        {
            m_exceptionPointer = FSM11STD::current_exception();
        }
    }
};

} // namespace fsm11

#endif // FSM11_THREADEDSTATE_HPP
