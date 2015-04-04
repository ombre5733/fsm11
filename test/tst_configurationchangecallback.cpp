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
#include "testutils.hpp"

using namespace fsm11;


namespace syncSM
{
using StateMachine_t = StateMachine<>;
using CallbackStateMachine_t = StateMachine<
                                   ConfigurationChangeCallbacksEnable<true>>;
using CallbackState_t = State<CallbackStateMachine_t>;
} // namespace syncSM


namespace asyncSM
{
using StateMachine_t = StateMachine<AsynchronousEventDispatching>;
using CallbackStateMachine_t = StateMachine<
                                   AsynchronousEventDispatching,
                                   ConfigurationChangeCallbacksEnable<true>>;
using CallbackState_t = State<CallbackStateMachine_t>;
} // namespace asyncSM

TEST_CASE("an FSM without configuration callback compiles only when no callback is set",
          "[callback]")
{
    syncSM::StateMachine_t sync_sm;
    // When the following line is included, the test must not compile.
    // sync_sm.setConfigurationChangeCallback([](){});

    asyncSM::StateMachine_t async_sm;
    // When the following line is included, the test must not compile.
    // async_sm.setConfigurationChangeCallback([](){});
}

TEST_CASE("disabling configuration callbacks makes the FSM smaller",
          "[callback]")
{
    REQUIRE(sizeof(syncSM::StateMachine_t)
            < sizeof(syncSM::CallbackStateMachine_t));
    REQUIRE(sizeof(asyncSM::StateMachine_t)
            < sizeof(asyncSM::CallbackStateMachine_t));
}

SCENARIO("configuration callback is invoked after starting and stopping",
         "[callback]")
{
    GIVEN ("a synchronous FSM")
    {
        using namespace syncSM;

        int numInvokes = 0;
        CallbackStateMachine_t sm;
        sm.setConfigurationChangeCallback([&](){ ++numInvokes; });

        REQUIRE(numInvokes == 0);
        REQUIRE(sm.numConfigurationChanges() == 0);

        WHEN ("the FSM is started")
        {
            sm.start();
            THEN ("the configuration callback is invoked")
            {
                REQUIRE(numInvokes == 1);
                REQUIRE(sm.numConfigurationChanges() == 1);
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("the configuration callback is invoked again")
                {
                    REQUIRE(numInvokes == 2);
                    REQUIRE(sm.numConfigurationChanges() == 2);
                }
            }
        }
    }

    GIVEN ("an asynchronous FSM")
    {
        using namespace asyncSM;

        std::future<void> result;

        std::mutex mutex;
        int numInvokes = 0;
        bool configurationChanged = false;
        std::condition_variable cv;

        auto waitForConfigurationChange = [&] {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&] { return configurationChanged; });
            configurationChanged = false;
        };

        CallbackStateMachine_t sm;
        sm.setConfigurationChangeCallback([&] {
            std::unique_lock<std::mutex> lock(mutex);
            configurationChanged = true;
            ++numInvokes;
            cv.notify_all();
        });

        REQUIRE(numInvokes == 0);
        REQUIRE(sm.numConfigurationChanges() == 0);

        WHEN ("the FSM is started")
        {
            result = sm.startAsyncEventLoop();
            sm.start();
            waitForConfigurationChange();
            THEN ("the configuration callback is invoked")
            {
                std::lock_guard<std::mutex> lock(mutex);
                REQUIRE(numInvokes == 1);
                REQUIRE(sm.numConfigurationChanges() == 1);
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                waitForConfigurationChange();
                THEN ("the configuration callback is invoked again")
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    REQUIRE(numInvokes == 2);
                    REQUIRE(sm.numConfigurationChanges() == 2);
                }
            }
        }
    }
}

SCENARIO("run-to-completion changes the configuration only once",
         "[callback]")
{
    GIVEN ("a synchronous FSM")
    {
        using namespace syncSM;

        int numInvokes = 0;
        CallbackStateMachine_t sm;
        CallbackState_t a("a", &sm);
        CallbackState_t b("b", &sm);
        CallbackState_t c("c", &sm);

        sm += a + event(1) > b;
        sm += b + noEvent > c;

        sm.setConfigurationChangeCallback([&](){ ++numInvokes; });
        sm.start();

        REQUIRE(numInvokes == 1);
        REQUIRE(sm.numConfigurationChanges() == 1);
        REQUIRE(isActive(sm, {&sm, &a}));

        WHEN ("an event is added")
        {
            sm.addEvent(1);
            THEN ("the configuration changes only after running to completion")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
                REQUIRE(numInvokes == 2);
                REQUIRE(sm.numConfigurationChanges() == 2);
            }
        }
    }

    GIVEN ("an asynchronous FSM")
    {
        using namespace asyncSM;

        std::future<void> result;

        std::mutex mutex;
        int numInvokes = 0;
        bool configurationChanged = false;
        std::condition_variable cv;

        auto waitForConfigurationChange = [&] {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&] { return configurationChanged; });
            configurationChanged = false;
        };

        CallbackStateMachine_t sm;
        CallbackState_t a("a", &sm);
        CallbackState_t b("b", &sm);
        CallbackState_t c("c", &sm);

        sm += a + event(1) > b;
        sm += b + noEvent > c;

        sm.setConfigurationChangeCallback([&] {
            std::unique_lock<std::mutex> lock(mutex);
            configurationChanged = true;
            ++numInvokes;
            cv.notify_all();
        });

        result = sm.startAsyncEventLoop();
        sm.start();
        waitForConfigurationChange();

        std::unique_lock<std::mutex> lock(mutex);
        REQUIRE(numInvokes == 1);
        REQUIRE(sm.numConfigurationChanges() == 1);
        REQUIRE(isActive(sm, {&sm, &a}));
        lock.unlock();

        WHEN ("an event is added")
        {
            sm.addEvent(1);
            waitForConfigurationChange();
            THEN ("the configuration changes only after running to completion")
            {
                lock.lock();
                REQUIRE(isActive(sm, {&sm, &c}));
                REQUIRE(numInvokes == 2);
                REQUIRE(sm.numConfigurationChanges() == 2);
            }
        }
    }
}

SCENARIO("configuration changes with respect to special transitions",
         "[callback]")
{
    GIVEN ("a state machine")
    {
        using namespace syncSM;

        int numInvokes = 0;
        CallbackStateMachine_t sm;
        CallbackState_t a("a", &sm);
        CallbackState_t b("b", &sm);
        CallbackState_t c("c", &sm);

        sm += a + event(1) > a;
        sm += a + event(2) > b;
        sm += a + event(3) > noTarget;
        sm += b + noEvent > c;
        sm += c + noEvent > a;

        sm.setConfigurationChangeCallback([&](){ ++numInvokes; });
        sm.start();

        REQUIRE(numInvokes == 1);
        REQUIRE(sm.numConfigurationChanges() == 1);
        REQUIRE(isActive(sm, {&sm, &a}));

        WHEN ("the source and the target of a transition are equal")
        {
            sm.addEvent(1);
            THEN ("the configuration changes once")
            {
                REQUIRE(numInvokes == 2);
                REQUIRE(sm.numConfigurationChanges() == 2);
            }
        }

        WHEN ("event-less transitions are encountered")
        {
            sm.addEvent(2);
            THEN ("the configuration changes once")
            {
                REQUIRE(numInvokes == 2);
                REQUIRE(sm.numConfigurationChanges() == 2);
            }
        }

        WHEN ("an event does not match a transition")
        {
            sm.addEvent(4);
            THEN ("the configuration does not change")
            {
                REQUIRE(numInvokes == 1);
                REQUIRE(sm.numConfigurationChanges() == 1);
            }
        }

    }
}

SCENARIO("configuration callbacks can be reset", "[callback]")
{
    GIVEN ("a state machine")
    {
        using namespace syncSM;

        int numInvokes = 0;
        CallbackStateMachine_t sm;
        CallbackState_t a("a", &sm);

        sm += a + event(1) > a;

        sm.setConfigurationChangeCallback([&](){ ++numInvokes; });
        sm.start();

        REQUIRE(numInvokes == 1);
        REQUIRE(sm.numConfigurationChanges() == 1);
        REQUIRE(isActive(sm, {&sm, &a}));

        WHEN ("an event is added")
        {
            sm.addEvent(1);
            THEN ("the configuration changes once")
            {
                REQUIRE(numInvokes == 2);
                REQUIRE(sm.numConfigurationChanges() == 2);
            }
        }

        WHEN ("the callback is reset")
        {
            sm.setConfigurationChangeCallback(nullptr);
            sm.addEvent(1);
            THEN ("the configuration counter increases but the callback is "
                  "not executed")
            {
                REQUIRE(numInvokes == 1);
                REQUIRE(sm.numConfigurationChanges() == 2);
            }
        }
    }
}
