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
            result = std::async(std::launch::async, [&] { sm.eventLoop(); });
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

        result = std::async(std::launch::async, [&] { sm.eventLoop(); });
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

        WHEN ("a target-less transition is encountered")
        {
            sm.addEvent(3);
            THEN ("the configuration does not change")
            {
                REQUIRE(numInvokes == 1);
                REQUIRE(sm.numConfigurationChanges() == 1);
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

            WHEN ("it is reset again")
            {
                sm.setConfigurationChangeCallback(nullptr);
                sm.addEvent(1);
                THEN ("this is a no-op")
                {
                    REQUIRE(numInvokes == 1);
                    REQUIRE(sm.numConfigurationChanges() == 3);
                }
            }

            WHEN ("it is turned on again")
            {
                sm.setConfigurationChangeCallback([&](){ ++numInvokes; });
                sm.addEvent(1);
                THEN ("it is invoked again")
                {
                    REQUIRE(numInvokes == 2);
                    REQUIRE(sm.numConfigurationChanges() == 3);
                }
            }
        }
    }
}
