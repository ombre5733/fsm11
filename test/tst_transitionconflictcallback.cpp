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
