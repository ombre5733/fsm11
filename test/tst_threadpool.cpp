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

#include "../src/statemachine.hpp"
#include "../src/threadedstate.hpp"
#include "../src/threadpool.hpp"

#include "testutils.hpp"

#include <utility>

using namespace fsm11;

template <typename TBaseState>
class TestState : public TBaseState
{
public:
    using TBaseState::TBaseState;

    virtual void invoke(fsm11::ExitRequest&) override
    {
        using namespace std;

        lock_guard<mutex> lock(m_mutex);
        m_id = this_thread::get_id();
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    std::thread::id threadId() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_id;
    }

private:
    mutable std::mutex m_mutex;
    std::thread::id m_id;
};

#if 0
TEST_CASE("thread pool construction and moving", "[threadpool]")
{
    SECTION("construction")
    {
        ThreadPool<10> pool;
    }

    SECTION("move construction")
    {
        ThreadPool<10> pool1;
        ThreadPool<10> pool2(std::move(pool1));
    }

    SECTION("move assignment")
    {
        ThreadPool<10> pool1;
        ThreadPool<10> pool2;
        pool2 = std::move(pool1);
    }
}

TEST_CASE("invoked actions are executed in a thread pool", "[threadpool]")
{
    GIVEN ("a synchronous FSM with a thread pool")
    {
        using StateMachine_t = StateMachine<ThreadPoolEnable<true, 3>>;
        using ThreadPool_t = StateMachine_t::thread_pool_type;
        using State_t = ThreadedState<StateMachine_t>;

        ThreadPool_t pool;
        StateMachine_t sm(std::move(pool));
        TestState<State_t> a("a", &sm);
        TestState<State_t> b("b", &a);
        TestState<State_t> c("c", &b);

        WHEN ("the FSM is started")
        {
            sm.start();
            sm.stop();

            THEN ("the invoked actions are started in the pool's threads")
            {
                for (auto state : {&a, &b, &c})
                {
                    REQUIRE(state->threadId() != std::thread::id());
                    REQUIRE(state->threadId() != std::this_thread::get_id());
                }
            }
        }
    }

    GIVEN ("an asynchronous FSM with a thread pool")
    {
        std::mutex mutex;
        bool configurationChanged = false;
        std::condition_variable cv;

        auto waitForConfigurationChange = [&] {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&] { return configurationChanged; });
            configurationChanged = false;
        };

        using StateMachine_t = StateMachine<AsynchronousEventDispatching,
                                            ThreadPoolEnable<true, 3>,
                                            ConfigurationChangeCallbacksEnable<true>>;
        using ThreadPool_t = StateMachine_t::thread_pool_type;
        using State_t = ThreadedState<StateMachine_t>;

        ThreadPool_t pool;
        StateMachine_t sm(std::move(pool));
        TestState<State_t> a("a", &sm);
        TestState<State_t> b("b", &a);
        TestState<State_t> c("c", &b);

        sm.setConfigurationChangeCallback([&] {
            std::unique_lock<std::mutex> lock(mutex);
            configurationChanged = true;
            cv.notify_all();
        });

        WHEN ("the FSM is started")
        {
            sm.start();
            auto result = sm.startAsyncEventLoop();
            waitForConfigurationChange();
            sm.stop();
            result.get();

            THEN ("the invoked actions are started in the pool's threads")
            {
                for (auto state : {&a, &b, &c})
                {
                    REQUIRE(state->threadId() != std::thread::id());
                    REQUIRE(state->threadId() != std::this_thread::get_id());
                }
            }
        }
    }
}
#endif

#include <iostream>
TEST_CASE("the thread pool throws an exception on underflow", "[threadpool]")
{
    GIVEN ("a synchronous FSM whose pool is too small")
    {
        using StateMachine_t = StateMachine<ThreadPoolEnable<true, 2>>;
        using ThreadPool_t = StateMachine_t::thread_pool_type;
        using State_t = ThreadedState<StateMachine_t>;

        ThreadPool_t pool;
        StateMachine_t sm(std::move(pool));
        TestState<State_t> a("a", &sm);
        TestState<State_t> b("b", &a);
        TestState<State_t> c("c", &b);

        WHEN ("the FSM is started")
        {
            THEN ("an underflow exception is thrown")
            {
                try
                {
                    sm.start();
                    REQUIRE(false);
                }
                catch (FsmError& error)
                {
                    REQUIRE(error.code() == FsmErrorCode::ThreadPoolUnderflow);
                }
                catch (...)
                {
                    REQUIRE(false);
                }
            }
        }
    }

    GIVEN ("an asynchronous FSM with a thread pool")
    {
        std::mutex mutex;
        bool configurationChanged = false;
        std::condition_variable cv;

        auto waitForConfigurationChange = [&] {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&] { return configurationChanged; });
            configurationChanged = false;
        };

        using StateMachine_t = StateMachine<AsynchronousEventDispatching,
                                            ThreadPoolEnable<true, 2>,
                                            ConfigurationChangeCallbacksEnable<true>>;
        using ThreadPool_t = StateMachine_t::thread_pool_type;
        using State_t = ThreadedState<StateMachine_t>;

        ThreadPool_t pool;
        StateMachine_t sm(std::move(pool));
        TestState<State_t> a("a", &sm);
        TestState<State_t> b("b", &a);
        TestState<State_t> c("c", &b);

        sm.setConfigurationChangeCallback([&] {
            std::unique_lock<std::mutex> lock(mutex);
            configurationChanged = true;
            cv.notify_all();
        });

        sm.start();
        auto result = sm.startAsyncEventLoop();
        waitForConfigurationChange();
        sm.stop();
        result.get();

#if 0
        WHEN ("the FSM is started")
        {
            THEN ("an underflow exception is thrown")
            {
                try
                {
                    REQUIRE(false);
                }
                catch (FsmError& error)
                {
                    REQUIRE(error.code() == FsmErrorCode::ThreadPoolUnderflow);
                }
                catch (...)
                {
                    REQUIRE(false);
                }
            }
        }
#endif
    }
}
