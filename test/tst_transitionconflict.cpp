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

SCENARIO("transition conflict callback", "[conflicts]")
{
    GIVEN ("that the transition selection stops after the first match")
    {
        using StateMachine_t = StateMachine<
                                   TransitionConflictPolicy<InvokeCallback>>;
        using State_t = State<StateMachine_t>;
        using Transition_t = Transition<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        int conflicts = 0;
        sm.setTransitionConflictCallback([&](Transition_t*, Transition_t*) {
                                             ++conflicts;
                                         });

        sm += a + event(1) > b;
        sm += a + event(1) > c;

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));

        WHEN ("a matching event is added")
        {
            sm.addEvent(1);
            THEN ("the callback is not invoked")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
                REQUIRE(conflicts == 0);
            }
        }
    }

    GIVEN ("that all transitions of a state are scanned")
    {
        using StateMachine_t = StateMachine<
                                   TransitionConflictPolicy<InvokeCallback>,
                                   TransitionSelectionStopsAfterFirstMatch<false>>;
        using State_t = State<StateMachine_t>;
        using Transition_t = Transition<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        Transition_t* t1 = sm += a + event(1) > b;
        Transition_t* t2 = sm += a + event(1) > c;

        int conflicts = 0;
        sm.setTransitionConflictCallback([&](Transition_t* a, Transition_t* b) {
            ++conflicts;
            REQUIRE(a == t1);
            REQUIRE(b == t2);
        });

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));

        WHEN ("a matching event is added")
        {
            sm.addEvent(1);
            THEN ("the conflict is reported via the callback")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
                REQUIRE(conflicts == 1);
            }
        }
    }
}

SCENARIO("transition conflict exception", "[conflicts]")
{
    GIVEN ("that the transition selection stops after the first match")
    {
        using StateMachine_t = StateMachine<
                                   TransitionConflictPolicy<ThrowException>>;
        using State_t = State<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        sm += a + event(1) > b;
        sm += a + event(1) > c;

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));

        WHEN ("a matching event is added")
        {
            sm.addEvent(1);
            THEN ("the exception is not thrown")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
            }
        }
    }

    GIVEN ("that all transitions of a state are scanned")
    {
        using StateMachine_t = StateMachine<
                                   TransitionConflictPolicy<ThrowException>,
                                   TransitionSelectionStopsAfterFirstMatch<false>>;
        using State_t = State<StateMachine_t>;
        using Transition_t = Transition<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        Transition_t* t1 = sm += a + event(1) > b;
        Transition_t* t2 = sm += a + event(1) > c;

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));

        WHEN ("a matching event is added")
        {
            THEN ("the conflict is reported via the exception")
            {
                try
                {
                    sm.addEvent(1);
                    REQUIRE(false);
                }
                catch (StateMachine_t::transition_conflict_error_type& error)
                {
                    REQUIRE(error.first() == t1);
                    REQUIRE(error.second() == t2);
                }
                catch (...)
                {
                    REQUIRE(false);
                }
                REQUIRE(!sm.running());
            }
        }
    }
}
