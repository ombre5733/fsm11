/*******************************************************************************
  fsm11 - A C++11-compliant framework for finite state machines

  Copyright (c) 2015, Manuel Freiberger
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

#include "catch.hpp"

#include "../src/threadedstate.hpp"
#include "../src/statemachine.hpp"

#include <condition_variable>
#include <mutex>
#include <thread>

using namespace fsm11;

namespace syncSM
{
using StateMachine_t = StateMachine<>;
using State_t = ThreadedState<StateMachine_t>;
} // namespace syncSM

namespace asyncSM
{
using StateMachine_t = StateMachine<AsynchronousEventDispatching,
                                    ConfigurationChangeCallbacksEnable<true>>;
using State_t = ThreadedState<StateMachine_t>;
} // namespace asyncSM

std::mutex g_mutex;
std::thread::id g_id;
std::condition_variable g_cv;
bool g_notify;
int g_invokeState;

template <typename TBaseState>
class TestState : public TBaseState
{
public:
    using TBaseState::TBaseState;

    virtual void invoke(fsm11::ExitRequest&) override
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_id = std::this_thread::get_id();
    }
};

template <typename TBaseState, unsigned TFn>
class WaitingState : public TBaseState
{
public:
    using TBaseState::TBaseState;

    virtual void invoke(fsm11::ExitRequest& request) override
    {
        g_mutex.lock();
        g_notify = true;
        g_invokeState = 1;
        g_mutex.unlock();
        g_cv.notify_all();

        switch (TFn)
        {
        case 0: request.wait(); break;
        case 1: request.waitFor(std::chrono::milliseconds(500)); break;
        default: REQUIRE(false); break;
        }

        g_mutex.lock();
        g_notify = true;
        g_invokeState = 2;
        g_mutex.unlock();
        g_cv.notify_all();
    }
};

TEST_CASE("construct threaded state", "[threadedstate]")
{
    using namespace syncSM;

    TestState<State_t> s1("s1");
    TestState<State_t> s2("s2", &s1);
    REQUIRE(s2.parent() == &s1);
}

TEST_CASE("start invoke action in threaded state", "[threadedstate]")
{
    using namespace syncSM;

    StateMachine_t sm;
    TestState<State_t> s1("s1", &sm);

    g_mutex.lock();
    g_id = std::this_thread::get_id();
    g_mutex.unlock();

    sm.start();
    sm.stop();
    REQUIRE(g_id != std::this_thread::get_id());
}

TEST_CASE("waitForExitRequest blocks an invoked action", "[threadedstate]")
{
    using namespace syncSM;

    StateMachine_t sm;
    WaitingState<State_t, 0> s1("s1", &sm);

    g_notify = false;
    g_invokeState = 0;

    sm.start();

    std::unique_lock<std::mutex> lock(g_mutex);
    g_cv.wait(lock, [&] { return g_notify; });
    REQUIRE(g_invokeState == 1);
    lock.unlock();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // The invoked action must still be blocked.
    lock.lock();
    g_cv.wait(lock, [&] { return g_notify; });
    REQUIRE(g_invokeState == 1);
    g_notify = false;
    lock.unlock();

    sm.stop();

    lock.lock();
    g_cv.wait(lock, [&] { return g_notify; });
    REQUIRE(g_invokeState == 2);
    lock.unlock();
}

TEST_CASE("waitForExitRequestFor blocks an invoked action", "[threadedstate]")
{
    using namespace syncSM;

    StateMachine_t sm;
    WaitingState<State_t, 1> s1("s1", &sm);

    g_notify = false;
    g_invokeState = 0;

    sm.start();

    std::unique_lock<std::mutex> lock(g_mutex);
    g_cv.wait(lock, [&] { return g_notify; });
    REQUIRE(g_invokeState == 1);
    g_notify = false;
    lock.unlock();

    SECTION("timeout")
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        lock.lock();
        g_cv.wait(lock, [&] { return g_notify; });
        REQUIRE(g_invokeState == 2);
        lock.unlock();

        sm.stop();
    }

    SECTION("no timeout")
    {
        sm.stop();

        lock.lock();
        g_cv.wait(lock, [&] { return g_notify; });
        REQUIRE(g_invokeState == 2);
        lock.unlock();
    }
}

TEST_CASE("invoked action is left when state machine is destructed",
          "[threadedstate]")
{
    using namespace syncSM;

    g_notify = false;
    g_invokeState = 0;
    std::unique_lock<std::mutex> lock(g_mutex, std::defer_lock);
    std::unique_ptr<WaitingState<State_t, 0>> s1;

    {
        StateMachine_t sm;
        // The state must be destructed after sm's destructor has been called.
        s1.reset(new WaitingState<State_t, 0>("s1", &sm));

        sm.start();

        lock.lock();
        g_cv.wait(lock, [&] { return g_notify; });
        REQUIRE(g_invokeState == 1);
        g_notify = false;
        lock.unlock();
    }

    lock.lock();
    g_cv.wait(lock, [&] { return g_notify; });
    REQUIRE(g_invokeState == 2);
    lock.unlock();
}

struct ThreadedInvokeException
{
};

#include <iostream>
using namespace std;

template <typename TBaseState>
struct ThrowingState : public TBaseState
{
public:
    using TBaseState::TBaseState;

    virtual void invoke(fsm11::ExitRequest&) override
    {
        if (stdException)
        {
            throw std::bad_alloc();
        }
        else
        {
#ifdef FSM11_USE_WEOS
            throw WEOS_EXCEPTION(ThreadedInvokeException());
#else
            throw ThreadedInvokeException();
#endif
        }
    }

    bool stdException{false};
};

SCENARIO("throwing an exception in a threaded state", "[threadedstate]")
{
    GIVEN ("a synchronous FSM")
    {
        using namespace syncSM;
        std::unique_ptr<ThrowingState<State_t>> s;
        StateMachine_t sm;
        s.reset(new ThrowingState<State_t>("s", &sm));

        WHEN ("a standard exception is thrown in the invoked action")
        {
            s->stdException = true;
            sm.start();
            THEN ("it arrives at the caller")
            {
                REQUIRE_THROWS_AS(sm.stop(), std::bad_alloc);
            }
        }

        WHEN ("a custom exception is thrown in the invoked action")
        {
            s->stdException = false;
            sm.start();
            THEN ("it arrives at the caller")
            {
                REQUIRE_THROWS_AS(sm.stop(), ThreadedInvokeException);
            }
        }
    }
}
