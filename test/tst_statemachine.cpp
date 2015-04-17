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

#include "../src/statemachine.hpp"

using namespace fsm11;


namespace syncSM
{
using StateMachine_t = StateMachine<>;
using State_t = StateMachine_t::state_type;

struct TrackingStateMachine : public StateMachine<>
{
    virtual void onEntry(int) override
    {
        ++entered;
    }

    virtual void onExit(int) override
    {
        ++left;
    }

    std::atomic_int entered{0};
    std::atomic_int left{0};
};

} // namespace syncSM

namespace asyncSM
{
using StateMachine_t = StateMachine<AsynchronousEventDispatching,
                                    ConfigurationChangeCallbacksEnable<true>>;
using State_t = StateMachine_t::state_type;

struct TrackingStateMachine : public StateMachine_t
{
    virtual void onEntry(int) override
    {
        ++entered;
    }

    virtual void onExit(int) override
    {
        ++left;
    }

    std::atomic_int entered{0};
    std::atomic_int left{0};
};

} // namespace asyncSM


TEST_CASE("construct a synchronous statemachine", "[statemachine]")
{
    using namespace syncSM;
    StateMachine_t sm;
    REQUIRE(!sm.running());
    REQUIRE(sm.numConfigurationChanges() == 0);
}

TEST_CASE("construct an asynchronous statemachine", "[statemachine]")
{
    using namespace asyncSM;
    StateMachine_t sm;
    REQUIRE(!sm.running());
    REQUIRE(sm.numConfigurationChanges() == 0);
}

TEST_CASE("the eventloop of an asynchronous statemachine is left upon destruction",
          "[statemachine]")
{
    std::future<void> result;

    {
        using namespace asyncSM;
        StateMachine_t sm;
        REQUIRE(!sm.running());
        REQUIRE(sm.numConfigurationChanges() == 0);

        result = sm.startAsyncEventLoop();
    }
}

TEST_CASE("start an empty synchronous statemachine", "[statemachine]")
{
    using namespace syncSM;
    StateMachine_t sm;
    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    for (int cnt = 0; cnt < 2; ++cnt)
    {
        sm.start();
        REQUIRE(sm.running());
        REQUIRE(sm.isActive());
        REQUIRE(sm.numConfigurationChanges() == 2 * cnt + 1);
        sm.stop();
        REQUIRE(!sm.running());
        REQUIRE(!sm.isActive());
        REQUIRE(sm.numConfigurationChanges() == 2 * cnt + 2);
    }
}

TEST_CASE("start an empty asynchronous statemachine", "[statemachine]")
{
    using namespace asyncSM;

    std::mutex mutex;
    bool configurationChanged = false;
    std::condition_variable cv;

    auto waitForConfigurationChange = [&] {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&] { return configurationChanged; });
        configurationChanged = false;
    };

    StateMachine_t sm;
    sm.setConfigurationChangeCallback([&] {
        std::unique_lock<std::mutex> lock(mutex);
        configurationChanged = true;
        cv.notify_all();
    });

    REQUIRE(!sm.running());
    for (int cnt = 0; cnt < 2; ++cnt)
    {
        auto result = sm.startAsyncEventLoop();

        sm.start();
        waitForConfigurationChange();
        REQUIRE(sm.running());
        REQUIRE(sm.isActive());
        REQUIRE(sm.numConfigurationChanges() == 2 * cnt + 1);
        sm.stop();
        waitForConfigurationChange();
        REQUIRE(!sm.running());
        REQUIRE(!sm.isActive());
        REQUIRE(sm.numConfigurationChanges() == 2 * cnt + 2);
    }
}

TEST_CASE("an asynchronous statemachine is stopped upon destruction",
          "[statemachine]")
{
    using namespace asyncSM;
    std::future<void> result;

    std::mutex mutex;
    bool configurationChanged = false;
    std::condition_variable cv;

    auto waitForConfigurationChange = [&] {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&] { return configurationChanged; });
        configurationChanged = false;
    };

    SECTION("without starting")
    {
        StateMachine_t sm;
        result = sm.startAsyncEventLoop();
    }

    SECTION("with starting")
    {
        StateMachine_t sm;
        result = sm.startAsyncEventLoop();
        sm.start();
    }

    SECTION("with starting and waiting for a configuration change")
    {
        StateMachine_t sm;
        sm.setConfigurationChangeCallback([&] {
            std::unique_lock<std::mutex> lock(mutex);
            configurationChanged = true;
            cv.notify_all();
        });

        result = sm.startAsyncEventLoop();
        sm.start();
        waitForConfigurationChange();
    }
}

SCENARIO("state machine actions are executed", "[statemachine]")
{
    GIVEN ("a synchronous FSM")
    {
        using namespace syncSM;

        TrackingStateMachine sm;

        WHEN ("the state machine is not started")
        {
            REQUIRE(!sm.running());
            THEN ("neither onEntry() nor onExit() is called")
            {
                REQUIRE(sm.entered == 0);
                REQUIRE(sm.left == 0);
            }
        }

        WHEN ("the state machine is started")
        {
            sm.start();
            THEN ("the state machine executes its onEntry() method")
            {
                REQUIRE(sm.running());
                REQUIRE(sm.entered == 1);
                REQUIRE(sm.left == 0);
            }

            WHEN ("the state machine is stopped")
            {
                sm.stop();
                THEN ("the state machine executes its onExit() method")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(sm.entered == 1);
                    REQUIRE(sm.left == 1);
                }
            }
        }
    }

    GIVEN ("an asynchronous FSM")
    {
        using namespace asyncSM;

        std::future<void> result;

        std::mutex mutex;
        bool configurationChanged = false;
        std::condition_variable cv;

        auto waitForConfigurationChange = [&] {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&] { return configurationChanged; });
            configurationChanged = false;
        };

        TrackingStateMachine sm;
        sm.setConfigurationChangeCallback([&] {
            std::unique_lock<std::mutex> lock(mutex);
            configurationChanged = true;
            cv.notify_all();
        });

        result = sm.startAsyncEventLoop();

        WHEN ("the state machine is not started")
        {
            REQUIRE(!sm.running());
            THEN ("neither onEntry() nor onExit() is called")
            {
                REQUIRE(sm.entered == 0);
                REQUIRE(sm.left == 0);
            }
        }

        WHEN ("the state machine is started")
        {
            sm.start();
            waitForConfigurationChange();
            THEN ("the state machine executes its onEntry() method")
            {
                REQUIRE(sm.running());
                REQUIRE(sm.entered == 1);
                REQUIRE(sm.left == 0);
            }

            WHEN ("the state machine is stopped")
            {
                sm.stop();
                waitForConfigurationChange();
                THEN ("the state machine executes its onExit() method")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(sm.entered == 1);
                    REQUIRE(sm.left == 1);
                }
            }
        }
    }
}

TEST_CASE("find a descendant of a state machine", "[statemachine]")
{
    using namespace syncSM;

    StateMachine_t sm;
    State_t p("p", &sm);
    State_t c1("c1", &p);
    State_t c2("c2", &p);
    State_t c3("c3", &p);
    State_t c11("c11", &c1);
    State_t c12("c12", &c1);
    State_t c31("c31", &c3);
    State_t c32("c32", &c3);

    REQUIRE(sm.findDescendant({}) == &sm);
    REQUIRE(sm.findDescendant({"p", "c1"}) == &c1);
    REQUIRE(sm.findDescendant({"p", "c3", "c32"}) == &c32);
    REQUIRE(sm.findDescendant({"p", "x"}) == nullptr);
    REQUIRE(sm.findDescendant({"x"}) == nullptr);
}
