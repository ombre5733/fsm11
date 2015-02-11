#ifndef FSM11_THREADEDSTATE_HPP
#define FSM11_THREADEDSTATE_HPP

#include "state.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/exception.hpp>
#include <weos/thread.hpp>
#else
#include <exception>
#include <thread>
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
#ifdef FSM11_USE_WEOS
                           weos::thread::attributes attrs,
#endif // FSM11_USE_WEOS
                           base_type* parent = 0)
        : base_type(name, parent)
#ifdef FSM11_USE_WEOS
        , m_invokeThreadAttributes(attrs)
#endif // FSM11_USE_WEOS
    {
    }

    //! \brief The actual invoke action.
    //!
    //! This method is called in a new thread. Derived classes have to
    //! override this function.
    virtual void invoke() = 0;

    virtual void enterInvoke() override
    {
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
        m_invokeThread.join();
        return m_exceptionPointer;
    }

private:
#ifdef FSM11_USE_WEOS
    weos::thread::attributes m_invokeThreadAttributes;
#endif // FSM11_USE_WEOS

    FSM11STD::thread m_invokeThread;
    FSM11STD::exception_ptr m_exceptionPointer;

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
