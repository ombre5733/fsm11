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
