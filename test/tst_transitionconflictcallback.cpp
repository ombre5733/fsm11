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


using StateMachine_t = StateMachine<>;
using CallbackStateMachine_t = StateMachine<
                                   TransitionConflictCallbacksEnable<true>,
                                   TransitionSelectionStopsAfterFirstMatch<false>>;
using CallbackState_t = State<CallbackStateMachine_t>;
using CallbackTransition_t = Transition<CallbackStateMachine_t>;

TEST_CASE("an FSM without transition conflict callback compiles only when "
          "no callback is set", "[callback]")
{
    StateMachine_t sm;
    // When one of the following lines is included, the test must not compile.
    //sm.setTransitionConflictCallback([](){});
}

TEST_CASE("disabling transition conflict callbacks makes the FSM smaller",
          "[callback]")
{
    REQUIRE(sizeof(StateMachine_t) < sizeof(CallbackStateMachine_t));
}

SCENARIO("transition conflict callback execution", "[callback]")
{
    GIVEN ("a state machine with a transition conflict")
    {
        int numInvokes = 0;
        CallbackStateMachine_t sm;
        CallbackState_t a("a", &sm);
        CallbackState_t b("b", &sm);
        using Transition_t = Transition<CallbackStateMachine_t>;

        sm += a + event(1) > a;
        sm += a + event(2) > a;
        sm += a + event(1) > b;

        sm.setTransitionConflictCallback([&](Transition_t*, Transition_t*){
                                             ++numInvokes;
                                         });
        sm.start();

        REQUIRE(numInvokes == 0);

        WHEN ("an event matching the conflict is added")
        {
            sm.addEvent(1);
            THEN ("the callback is called once")
            {
                REQUIRE(numInvokes == 1);
            }
        }

        WHEN ("an event, which does not match the conflict, is added")
        {
            sm.addEvent(2);
            THEN ("the callback is not invoked")
            {
                REQUIRE(numInvokes == 0);
            }
        }

        WHEN ("the callback is reset")
        {
            sm.setTransitionConflictCallback(nullptr);
            sm.addEvent(1);
            THEN ("the callback is not invoked")
            {
                REQUIRE(numInvokes == 0);
            }

            WHEN ("the callback is reset twice")
            {
                sm.setTransitionConflictCallback(nullptr);
                sm.addEvent(1);
                THEN ("this is a no-op")
                {
                    REQUIRE(numInvokes == 0);
                }
            }

            WHEN ("the callback is turned on again")
            {
                sm.setTransitionConflictCallback([&](Transition_t*, Transition_t*){
                                                     ++numInvokes;
                                                 });
                sm.addEvent(1);
                THEN ("it is invoked again")
                {
                    REQUIRE(numInvokes == 1);
                }
            }
        }
    }
}
