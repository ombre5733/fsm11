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
