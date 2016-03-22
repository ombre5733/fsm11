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
                                   EventCallbacksEnable<true>>;
using CallbackState_t = State<CallbackStateMachine_t>;
} // namespace syncSM

namespace asyncSM
{
using StateMachine_t = StateMachine<AsynchronousEventDispatching>;
using CallbackStateMachine_t = StateMachine<
                                   AsynchronousEventDispatching,
                                   EventCallbacksEnable<true>>;
} // namespace asyncSM

TEST_CASE("an FSM without event callback compiles only when no callback is set",
          "[callback]")
{
    syncSM::StateMachine_t sync_sm;
    // When one of the following lines is included, the test must not compile.
    // sync_sm.setEventDispatchCallback([](int){});
    // sync_sm.setEventDiscardedCallback([](int){});

    asyncSM::StateMachine_t async_sm;
    // When one of the following lines is included, the test must not compile.
    // async_sm.setEventDispatchCallback([](int){});
    // async_sm.setEventDiscardedCallback([](int){});
}

TEST_CASE("disabling event callbacks makes the FSM smaller", "[callback]")
{
    REQUIRE(sizeof(syncSM::StateMachine_t)
            < sizeof(syncSM::CallbackStateMachine_t));
    REQUIRE(sizeof(asyncSM::StateMachine_t)
            < sizeof(asyncSM::CallbackStateMachine_t));
}

SCENARIO("event callback execution", "[callback]")
{
    GIVEN ("a state machine")
    {
        using namespace syncSM;

        int numDispatchedEvents = 0;
        int numDiscardedEvents = 0;
        CallbackStateMachine_t sm;
        CallbackState_t a("a", &sm);

        sm += a + event(1) > a;

        sm.setEventDispatchCallback([&](int){ ++numDispatchedEvents; });
        sm.setEventDiscardedCallback([&](int){ ++ numDiscardedEvents; });
        sm.start();

        REQUIRE(numDispatchedEvents == 0);
        REQUIRE(numDiscardedEvents == 0);

        WHEN ("a matching event is added")
        {
            sm.addEvent(1);
            THEN ("the numer of dispatched events increases")
            {
                REQUIRE(numDispatchedEvents == 1);
                REQUIRE(numDiscardedEvents == 0);
            }
        }

        WHEN ("a non-matching event is added")
        {
            sm.addEvent(2);
            THEN ("the numer of dispatched and discarded events increase")
            {
                REQUIRE(numDispatchedEvents == 1);
                REQUIRE(numDiscardedEvents == 1);
            }
        }

        WHEN ("the callbacks are reset")
        {
            sm.setEventDispatchCallback(nullptr);
            sm.setEventDiscardedCallback(nullptr);
            sm.addEvent(1);
            sm.addEvent(2);
            THEN ("the counters do not increase")
            {
                REQUIRE(numDispatchedEvents == 0);
                REQUIRE(numDiscardedEvents == 0);
            }

            WHEN ("the callbacks are reset twice")
            {
                sm.setEventDispatchCallback(nullptr);
                sm.setEventDiscardedCallback(nullptr);
                sm.addEvent(1);
                sm.addEvent(2);
                THEN ("this is a no-op")
                {
                    REQUIRE(numDispatchedEvents == 0);
                    REQUIRE(numDiscardedEvents == 0);
                }
            }

            WHEN ("the callbacks are turned on again")
            {
                sm.setEventDispatchCallback([&](int){ ++numDispatchedEvents; });
                sm.setEventDiscardedCallback([&](int){ ++ numDiscardedEvents; });
                sm.addEvent(1);
                sm.addEvent(2);
                THEN ("they are invoked again")
                {
                    REQUIRE(numDispatchedEvents == 2);
                    REQUIRE(numDiscardedEvents == 1);
                }
            }
        }
    }
}
