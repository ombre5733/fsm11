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

#ifndef FSM11_EXITREQUEST_HPP
#define FSM11_EXITREQUEST_HPP

#include "statemachine_fwd.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/chrono.hpp>
#include <weos/condition_variable.hpp>
#include <weos/mutex.hpp>
#else
#include <chrono>
#include <condition_variable>
#include <mutex>
#endif // FSM11_USE_WEOS

namespace fsm11
{

class ExitRequest
{
public:
    ExitRequest()
        : m_requested(false)
    {
    }

    ExitRequest(const ExitRequest&) = delete;
    ExitRequest& operator=(const ExitRequest&) = delete;

    //! Waits for an exit request.
    void wait()
    {
        FSM11STD::unique_lock<FSM11STD::mutex> lock(m_mutex);
        m_cv.wait(lock, [&] { return m_requested; });
    }

    //! Waits for an exit request with a timeout.
    //!
    //! Waits for an exit request or until the \p timeout is expired.
    template <typename TRep, typename TPeriod>
    bool waitFor(const FSM11STD::chrono::duration<TRep, TPeriod>& timeout)
    {
        FSM11STD::unique_lock<FSM11STD::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, timeout, [&] { return m_requested; });
    }

    //! Checks if an exit has been requested.
    //!
    //! Returns \p true if an exit has been requested.
    explicit operator bool() const
    {
        FSM11STD::lock_guard<FSM11STD::mutex> lock(m_mutex);
        return m_requested;
    }

private:
    mutable FSM11STD::mutex m_mutex;
    FSM11STD::condition_variable m_cv;
    bool m_requested;


    template <typename TStateMachine>
    friend class ThreadedState;

    template <typename TStateMachine>
    friend class ThreadedFunctionState;
};

} // namespace fsm11

#endif // FSM11_EXITREQUEST_HPP
