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
