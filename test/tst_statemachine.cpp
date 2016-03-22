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

#include "catch.hpp"

#include "../src/statemachine.hpp"
#include <functional>

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
    using namespace asyncSM;

    std::future<void> result;
    StateMachine_t sm;
    REQUIRE(!sm.running());
    REQUIRE(sm.numConfigurationChanges() == 0);

    result = std::async(std::launch::async, [&] { sm.eventLoop(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
        auto result = std::async(std::launch::async, [&] { sm.eventLoop(); });

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
        result = std::async(std::launch::async, [&] { sm.eventLoop(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    SECTION("with starting")
    {
        StateMachine_t sm;
        result = std::async(std::launch::async, [&] { sm.eventLoop(); });
        sm.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    SECTION("with starting and waiting for a configuration change")
    {
        StateMachine_t sm;
        sm.setConfigurationChangeCallback([&] {
            std::unique_lock<std::mutex> lock(mutex);
            configurationChanged = true;
            cv.notify_all();
        });

        result = std::async(std::launch::async, [&] { sm.eventLoop(); });
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

        result = std::async(std::launch::async, [&] { sm.eventLoop(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

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

struct BaseStateMachine : public StateMachine<>
{
    BaseStateMachine()
        : a("a", this),
          b("b", this)
    {
        *this += a + event(1)
                   [ std::bind(&BaseStateMachine::guard, this,
                               std::placeholders::_1) ]
                   / std::bind(&BaseStateMachine::action, this,
                               std::placeholders::_1)
                > b;
    }

    virtual ~BaseStateMachine()
    {
    }

    virtual bool guard(int) = 0;
    virtual void action(int) = 0;

    State<StateMachine<>> a;
    State<StateMachine<>> b;
};

struct DerivedStateMachine : public BaseStateMachine
{
    DerivedStateMachine()
        : enabled(false),
          numActions(0)
    {
    }

    virtual bool guard(int) override
    {
        return enabled;
    }

    virtual void action(int) override
    {
        ++numActions;
    }

    bool enabled;
    int numActions;
};

TEST_CASE("subclassing a state machine", "[statemachine]")
{
    DerivedStateMachine d;
    d.start();
    d.addEvent(1);
    REQUIRE(d.numActions == 0);
    d.enabled = true;
    d.addEvent(1);
    REQUIRE(d.numActions == 1);
}
